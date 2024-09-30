// Fill out your copyright notice in the Description page of Project Settings.


#include "DrawHandyManSpline.h"

#include "AssetSelection.h"
#include "SplineUtil.h"
#include "InteractiveToolManager.h"
#include "ToolBuilderUtil.h"
#include "UnrealEdGlobals.h"
#include "ActorFactories/ActorFactoryEmptyActor.h"
#include "BaseBehaviors/SingleClickOrDragBehavior.h"
#include "BaseGizmos/GizmoMath.h"
#include "Drawing/PreviewGeometryActor.h"
#include "Editor/UnrealEdEngine.h"
#include "Mechanics/ConstructionPlaneMechanic.h"
#include "Selection/ToolSelectionUtil.h"


#define LOCTEXT_NAMESPACE "UDrawHandyManSpline"

using namespace UE::Geometry;

UDrawHandyManSpline::UDrawHandyManSpline()
{
	ToolName = LOCTEXT("SplineToolName", "Draw Spline");
	ToolLongName = LOCTEXT("SplineToolName", "Draw Spline");
	ToolCategory = LOCTEXT("SplineToolName", "Splines");
	ToolTooltip = LOCTEXT("SplineToolName", "Draw a spline leaving behind a spline actor that can be used for other tools.");
}



void UDrawHandyManSpline::Setup()
{
	Super::Setup();

	Settings = NewObject<UDrawHandyManSplineProperties>(this);
	Settings->RestoreProperties(this);
	AddToolPropertySource(Settings);

	Settings->TargetActor = SelectedActor;

	SetToolDisplayName(LOCTEXT("DrawSplineToolName", "Draw Spline"));
	GetToolManager()->DisplayMessage(
		LOCTEXT("DrawSplineToolDescription", "Draw a spline to replace an existing one or add it to an actor."),
		EToolMessageLevel::UserNotification);

	PlaneMechanic = NewObject<UConstructionPlaneMechanic>(this);
	PlaneMechanic->Setup(this);
	PlaneMechanic->Initialize(GetToolWorld(), FFrame3d(FVector3d::Zero(), FVector3d::UnitX()));
	PlaneMechanic->bShowGrid = Settings->bHitCustomPlane;
	PlaneMechanic->CanUpdatePlaneFunc = [this] { return Settings->bHitCustomPlane; };
	Settings->WatchProperty(Settings->bHitCustomPlane, [this](bool) {
		PlaneMechanic->bShowGrid = Settings->bHitCustomPlane;
	});

	ClickOrDragBehavior = NewObject<USingleClickOrDragInputBehavior>();
	ClickOrDragBehavior->Initialize(this, this);
	AddInputBehavior(ClickOrDragBehavior);

	// Make sure the plane mechanic captures clicks first, to ensure it sees ctrl+clicks to reposition the plane
	PlaneMechanic->UpdateClickPriority(ClickOrDragBehavior->GetPriority().MakeHigher());

	Settings->WatchProperty(Settings->bLoop, [this](bool) {
		if (ensure(WorkingSpline.IsValid()))
		{
			WorkingSpline->SetClosedLoop(Settings->bLoop);
		}
	});

	TransitionOutputMode();

	Settings->WatchProperty(Settings->OutputMode, [this](EDrawSplineOutputMode_HandyMan) {
		TransitionOutputMode();
	});
	Settings->WatchProperty(Settings->TargetActor, [this](TWeakObjectPtr<AActor>) {
		TransitionOutputMode();
	});
	Settings->WatchProperty(Settings->ExistingSplineIndexToReplace, [this](int32) {
		TransitionOutputMode();
	});
	Settings->WatchProperty(Settings->BlueprintToCreate, [this](TWeakObjectPtr<UBlueprint>) {
		TransitionOutputMode();
		});
	Settings->WatchProperty(Settings->bPreviewUsingActorCopy, [this](bool) {
		TransitionOutputMode();
	});

	Settings->SilentUpdateWatched();
}

// Set things up for a new output mode or destination
void UDrawHandyManSpline::TransitionOutputMode()
{
	using namespace HandyMan::SplineUtils;

	// Setting up the previews seems to be the most error prone part of the tool because editor duplicating, hiding
	// from outliner, and avoiding emitting undo/redo transactions seems to be quite finnicky...

	// This function is sometimes called from inside transactions (such as tool start, or dragging the "component to replace"
	// slider). Several calls here would transact in that case (for instance, the Destroy() calls on the previews seem
	// to do it), which we generally don't want to do. So we disable transacting in this function with the hack below.
	// Note that we still have to take care that any editor functions we call don't open their own transactions...
	ITransaction* UndoState = GUndo;
	GUndo = nullptr; // Pretend we're not in a transaction
	ON_SCOPE_EXIT{ GUndo = UndoState; }; // Revert later

	// Restore the visibility of the previous target actor and spline, if needed
	if (PreviousTargetActor)
	{
		PreviousTargetActor->SetIsTemporarilyHiddenInEditor(PreviousTargetActorVisibility);
		PreviousTargetActor = nullptr;
	}
	if (HiddenSpline.IsValid())
	{
		HiddenSpline->bDrawDebug = bPreviousSplineVisibility;
		HiddenSpline = nullptr;
	}

	// Keep the previous spline/previews temporarily so we can transfer over spline data
	// when we make new previews
	AActor* PreviousPreviewRoot = PreviewRootActor;
	AActor* PreviousPreview = PreviewActor;
	USplineComponent* PreviousSpline = WorkingSpline.Get();

	PreviewRootActor = nullptr;
	PreviewActor = nullptr;
	WorkingSpline = nullptr;

	// Create an entirely new preview root. We could probably keep the same one and disconnect/connect,
	// but it seems cleaner to build from scratch whenever we have to change output mode.
	FRotator Rotation(0.0f, 0.0f, 0.0f);
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.ObjectFlags = RF_Transient;
	PreviewRootActor = GetToolWorld()->SpawnActor<APreviewGeometryActor>(FVector::ZeroVector, Rotation, SpawnInfo);
	USceneComponent* RootComponent = NewObject<USceneComponent>(PreviewRootActor);
	PreviewRootActor->AddOwnedComponent(RootComponent);
	PreviewRootActor->SetRootComponent(RootComponent);
	RootComponent->RegisterComponent();
	
	// Used for visualizing the effect of a spline on some special actor
	auto CreateDuplicatePreviewActor = [this](AActor* Actor)
	{
		TArray<AActor*> NewActors;
		GUnrealEd->DuplicateActors({ Actor }, NewActors, GetWorld()->GetCurrentLevel(), FVector3d::Zero());
		if (!ensure(NewActors.Num() > 0))
		{
			return;
		}

		PreviewActor = NewActors[0];
		PreviewActor->ClearFlags(RF_Transactional);
		PreviewActor->SetFlags(RF_Transient);

		if (!ensure(PreviewActor->GetRootComponent()))
		{
			USceneComponent* NewRoot = NewObject<USceneComponent>(PreviewRootActor);
			PreviewActor->AddOwnedComponent(NewRoot);
			PreviewActor->SetRootComponent(NewRoot);
			NewRoot->RegisterComponent();
		}

		// Attach the preview actor to the non-outliner-visible preview root. The proper way to do this is 
		// "GEditor->ParentActors(PreviewActor, PreviewRootActor, NAME_None);", but it is hard to prevent
		// that call from emitting an undo/redo transaction. It may be possible if the actors AND the root
		// components are all marked as not transactable, but seems simpler to do this by hand.
		PreviewActor->GetRootComponent()->AttachToComponent(PreviewRootActor->GetRootComponent(), 
			FAttachmentTransformRules::KeepWorldTransform, NAME_None);

		// Hide the original
		PreviousTargetActor = Actor;
		PreviousTargetActorVisibility = PreviousTargetActor->IsHiddenEd();
		PreviousTargetActor->SetIsTemporarilyHiddenInEditor(true);
	};

	auto FallbackSplinePlacement = [this]()
	{
		WorkingSpline = CreateNewSplineInActor(PreviewRootActor);
	};

	// Set up the new preview
	if (!Settings->bPreviewUsingActorCopy)
	{
		FallbackSplinePlacement();
	}
	else
	{
		switch (Settings->OutputMode)
		{
		case EDrawSplineOutputMode_HandyMan::EmptyActor:
			FallbackSplinePlacement();
			break;
		case EDrawSplineOutputMode_HandyMan::ExistingActor:
		{
			if (!Settings->TargetActor.IsValid())
			{
				FallbackSplinePlacement();
				break;
			}

			CreateDuplicatePreviewActor(Settings->TargetActor.Get());

			// Hide the spline we're replacing, if we are replacing one.
			// TODO: This isn't quite perfect because if the spline is selected, the component visualizer in the editor
			// will still draw it even if the parent actor is hidden and the spline is set to not be drawn...
			TInlineComponentArray<USplineComponent*> SplineComponents;
			Settings->TargetActor->GetComponents<USplineComponent>(SplineComponents);
			if (Settings->ExistingSplineIndexToReplace >= 0 && Settings->ExistingSplineIndexToReplace < SplineComponents.Num())
			{
				HiddenSpline = SplineComponents[Settings->ExistingSplineIndexToReplace];
				bPreviousSplineVisibility = HiddenSpline->bDrawDebug;
				HiddenSpline->bDrawDebug = false;
			}

			WorkingSpline = GetOrCreateTargetSpline(PreviewActor, Settings->ExistingSplineIndexToReplace);
			break;
		}
		case EDrawSplineOutputMode_HandyMan::CreateBlueprint:
		{
			bool bCanCreateActor = Settings->BlueprintToCreate.IsValid()
				&& Settings->BlueprintToCreate->GeneratedClass != nullptr
				&& !Settings->BlueprintToCreate->GeneratedClass->HasAnyClassFlags(CLASS_NotPlaceable | CLASS_Abstract);

			if (!bCanCreateActor)
			{
				FallbackSplinePlacement();
				break;
			}

			// Instantiate the blueprint
			PreviewActor = FActorFactoryAssetProxy::AddActorForAsset(
				Settings->BlueprintToCreate.Get(),
				/*bSelectActors =*/ false,
				// Important that we don't use the default (RF_Transactional) here, or else we'll end up
				// issuing an undo transaction in this call.
				EObjectFlags::RF_Transient);
			if (!PreviewActor)
			{
				FallbackSplinePlacement();
				break;
			}

			WorkingSpline = GetOrCreateTargetSpline(PreviewActor, Settings->ExistingSplineIndexToReplace);
			break;
		}
		default:
			ensure(false);
		}
	}

	if (ensure(WorkingSpline.IsValid()))
	{
		if (PreviousSpline)
		{
			CopySplineToSpline(*PreviousSpline, *WorkingSpline);
		}
		else
		{
			WorkingSpline->ClearSplinePoints();
		}

		WorkingSpline->SetClosedLoop(Settings->bLoop);

		// This has to be set so that construction script reruns transfer over current spline state.
		WorkingSpline->bSplineHasBeenEdited = true;

		// Get the index of the spline in the components array for recapturing on construction script reruns.
		if (PreviewActor)
		{
			TInlineComponentArray<USplineComponent*> SplineComponents;
			PreviewActor->GetComponents<USplineComponent>(SplineComponents);
			SplineRecaptureIndex = SplineComponents.IndexOfByKey(WorkingSpline.Get());
			ensure(SplineRecaptureIndex >= 0);
		}
	}
	
	// Now that we've copied over previous preview data, destroy the old previews
	if (PreviousPreview)
	{
		PreviousPreview->Destroy();
	}
	if (PreviousPreviewRoot)
	{
		PreviousPreviewRoot->Destroy();
	}
}

void UDrawHandyManSpline::Shutdown(EToolShutdownType ShutdownType)
{
	LongTransactions.CloseAll(GetToolManager());

	Settings->SaveProperties(this);

	if (PreviousTargetActor)
	{
		PreviousTargetActor->SetIsTemporarilyHiddenInEditor(PreviousTargetActorVisibility);
		PreviousTargetActor = nullptr;
	}
	if (HiddenSpline.IsValid())
	{
		HiddenSpline->bDrawDebug = bPreviousSplineVisibility;
		HiddenSpline = nullptr;
	}

	if (ShutdownType == EToolShutdownType::Accept && WorkingSpline.IsValid() && WorkingSpline->GetNumberOfSplinePoints() > 0)
	{
		GenerateAsset();
	}

	PlaneMechanic->Shutdown();
	
	
	if (WorkingSpline.IsValid())
	{
		WorkingSpline->DestroyComponent();
	}

	auto DestroyActor = [&](AActor* Actor)
	{
		if (UWorld* ActorWorld = Actor->GetWorld())
		{
			if (GIsEditor && GUnrealEd)
			{
				GUnrealEd->DeleteActors(TArray{ Actor }, ActorWorld, GUnrealEd->GetSelectedActors()->GetElementSelectionSet());
			}
			else
			{
				ActorWorld->DestroyActor(Actor);
			}
		}
	};
	if (PreviewActor)
	{
		DestroyActor(PreviewActor);
	}
	if (PreviewRootActor)
	{
		DestroyActor(PreviewRootActor);
	}

	Super::Shutdown(ShutdownType);
}

void UDrawHandyManSpline::GenerateAsset()
{
	using namespace HandyMan::SplineUtils;
	USplineComponent* OutputSpline = nullptr;
	auto CreateSplineInEmptyActor = [this, &OutputSpline]()
	{
		// Get centroid of spline
		int32 NumSplinePoints = WorkingSpline->GetNumberOfSplinePoints();
		FVector3d Center = FVector3d::Zero();
		for (int32 i = 0; i < NumSplinePoints; ++i)
		{
			Center += WorkingSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		}
		Center /= NumSplinePoints;

		// Spawning via a factory is editor-only
		UActorFactoryEmptyActor* EmptyActorFactory = NewObject<UActorFactoryEmptyActor>();
		FAssetData AssetData(EmptyActorFactory->GetDefaultActorClass(FAssetData()));
		FActorSpawnParameters SpawnParams;
		FString name = FString::Format(TEXT("Actor_{0}"), { WorkingSpline.Get()->GetFName().ToString() });
		FName UniqueName = MakeUniqueObjectName(nullptr, USplineComponent::StaticClass(), FName(*name));
		SpawnParams.Name = UniqueName;;
		SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
		AActor* NewActor = EmptyActorFactory->CreateActor(AssetData.GetAsset(),
			GetToolWorld()->GetCurrentLevel(),
			FTransform(Center),
			SpawnParams);

		// This is also editor-only: it's the label that shows up in the hierarchy
		FActorLabelUtilities::SetActorLabelUnique(NewActor, UniqueName.ToString());

		UActorComponent* OldRoot = NewActor->GetRootComponent();
		OutputSpline = CreateNewSplineInActor(NewActor, true, true);
		OutputSpline->SetWorldTransform(FTransform(Center));
		OldRoot->DestroyComponent();

		CopySplineToSpline(*WorkingSpline, *OutputSpline, true);
	};

	GetToolManager()->BeginUndoTransaction(LOCTEXT("DrawSplineTransactionName", "Draw Spline"));

	switch (Settings->OutputMode)
	{
	case EDrawSplineOutputMode_HandyMan::EmptyActor:
		CreateSplineInEmptyActor();
		break;
	case EDrawSplineOutputMode_HandyMan::ExistingActor:
	{
		if (!Settings->TargetActor.IsValid())
		{
			CreateSplineInEmptyActor();
			break;
		}

		OutputSpline = GetOrCreateTargetSpline(Settings->TargetActor.Get(), Settings->ExistingSplineIndexToReplace, true);
		CopySplineToSpline(*WorkingSpline, *OutputSpline, true);
		Settings->TargetActor->RerunConstructionScripts();
		break;
	}
	case EDrawSplineOutputMode_HandyMan::CreateBlueprint:
	{
		bool bCanCreateActor = Settings->BlueprintToCreate.IsValid()// != nullptr
			&& Settings->BlueprintToCreate->GeneratedClass != nullptr
			&& !Settings->BlueprintToCreate->GeneratedClass->HasAnyClassFlags(CLASS_NotPlaceable | CLASS_Abstract);

		if (!bCanCreateActor)
		{
			CreateSplineInEmptyActor();
			break;
		}

		// Instantiate the blueprint
		AActor* NewActor = FActorFactoryAssetProxy::AddActorForAsset(
			Settings->BlueprintToCreate.Get(),
			/*bSelectActors =*/ false);
		if (!NewActor)
		{
			CreateSplineInEmptyActor();
			break;
		}

		OutputSpline = GetOrCreateTargetSpline(NewActor, Settings->ExistingSplineIndexToReplace, true);
		CopySplineToSpline(*WorkingSpline, *OutputSpline, true);
		NewActor->RerunConstructionScripts();
		break;
	}
	default:
		ensure(false);
	}

	// TODO: Someday when we support component selection, we should select OutputSpline directly.
	ToolSelectionUtil::SetNewActorSelection(GetToolManager(), OutputSpline->GetAttachmentRootActor());

	GetToolManager()->EndUndoTransaction();
}

// Helper to add a point given a hit location and hit normal
void UDrawHandyManSpline::AddSplinePoint(const FVector3d& HitLocation, const FVector3d& HitNormal)
{
	using namespace HandyMan::SplineUtils;
	if (!WorkingSpline.IsValid())
	{
		return;
	}

	int32 NumSplinePoints = WorkingSpline->GetNumberOfSplinePoints();
	FVector3d UpVectorToUse = GetUpVectorToUse(HitLocation, HitNormal, NumSplinePoints);

	WorkingSpline->AddSplinePoint(HitLocation, ESplineCoordinateSpace::World, 
		/*bUpdate =*/ false);
	WorkingSpline->SetUpVectorAtSplinePoint(NumSplinePoints, UpVectorToUse, ESplineCoordinateSpace::World, 
		/*bUpdate =*/ true);
}

FVector3d UDrawHandyManSpline::GetUpVectorToUse(const FVector3d& HitLocation, const FVector3d& HitNormal, int32 NumSplinePointsBeforehand)
{
	FVector3d UpVectorToUse = HitNormal;
	switch (Settings->UpVectorMode)
	{
	case EDrawSplineUpVectorMode_HandyMan::AlignToPrevious:
	{
		if (NumSplinePointsBeforehand == 0)
		{
			// TODO: Maybe add some different options of what normal to start with
		}
		else if (NumSplinePointsBeforehand > 1)
		{
			UpVectorToUse = WorkingSpline->GetUpVectorAtSplinePoint(NumSplinePointsBeforehand - 1, ESplineCoordinateSpace::World);
		}
		else // if NumSplinePointsBeforehand == 1
		{
			// If there's only one point, GetUpVectorAtSplinePoint is unreliable because it seeks to build a
			// quaternion from the tangent and the set up vector, and the tangent is zero. We want to use
			// the "stored" up vector directly.
			FVector3d LocalUpVector = WorkingSpline->SplineCurves.Rotation.Points[0].OutVal.RotateVector(WorkingSpline->DefaultUpVector);
			UpVectorToUse = WorkingSpline->GetComponentTransform().TransformVectorNoScale(LocalUpVector);
		}
		break;
	}
	case EDrawSplineUpVectorMode_HandyMan::UseHitNormal:
		break;
	}

	return UpVectorToUse;
}

bool UDrawHandyManSpline::Raycast(const FRay& WorldRay, FVector3d& HitLocationOut, FVector3d& HitNormalOut, double& HitTOut)
{
	double BestHitT = TNumericLimits<double>::Max();
	
	if (Settings->bHitCustomPlane)
	{
		FVector IntersectionPoint;
		bool bHitPlane = false;
		GizmoMath::RayPlaneIntersectionPoint(PlaneMechanic->Plane.Origin, PlaneMechanic->Plane.Z(),
			WorldRay.Origin, WorldRay.Direction, bHitPlane, IntersectionPoint);

		if (bHitPlane)
		{
			HitLocationOut = IntersectionPoint;
			HitNormalOut = PlaneMechanic->Plane.Z();
			HitTOut = WorldRay.GetParameter(IntersectionPoint);
			BestHitT = HitTOut;
		}
	}

	if (Settings->bHitWorld)
	{
		FHitResult GeometryHit;
		TArray<const UPrimitiveComponent*> ComponentsToIgnore;
		if (PreviewActor)
		{
			PreviewActor->GetComponents<const UPrimitiveComponent>(ComponentsToIgnore);
		}
		if (ToolSceneQueriesUtil::FindNearestVisibleObjectHit(this, GeometryHit, WorldRay, &ComponentsToIgnore)
			&& GeometryHit.Distance < BestHitT)
		{
			HitLocationOut = GeometryHit.ImpactPoint;
			HitNormalOut = GeometryHit.ImpactNormal;
			HitTOut = GeometryHit.Distance;
			BestHitT = HitTOut;
		}
	}

	// Only raycast the ground plane / ortho background if we didn't hit anything else
	if (Settings->bHitGroundPlanes && BestHitT == TNumericLimits<double>::Max())
	{
		FVector3d PlaneNormal = CameraState.bIsOrthographic ? -WorldRay.Direction : FVector3d::UnitZ();
		FVector IntersectionPoint;
		bool bHitPlane = false;
		GizmoMath::RayPlaneIntersectionPoint(FVector3d::Zero(), PlaneNormal,
			WorldRay.Origin, WorldRay.Direction, bHitPlane, IntersectionPoint);

		if (bHitPlane)
		{
			HitLocationOut = IntersectionPoint;
			HitNormalOut = PlaneNormal;
			HitTOut = WorldRay.GetParameter(IntersectionPoint);
			BestHitT = HitTOut;
		}
	}

	if (Settings->ClickOffset != 0.0)
	{
		FVector3d OffsetDirection = HitNormalOut;
		if (Settings->OffsetMethod == ESplineOffsetMethod_HandyMan::Custom)
		{
			OffsetDirection = Settings->OffsetDirection.GetSafeNormal(UE_SMALL_NUMBER, FVector3d::UnitZ());
		}

		HitLocationOut += OffsetDirection * Settings->ClickOffset;
	}
	
	return BestHitT < TNumericLimits<double>::Max();
}

FInputRayHit UDrawHandyManSpline::IsHitByClick(const FInputDeviceRay& ClickPos)
{
	FVector3d HitLocation, HitNormal;
	double HitT;
	if (Raycast(ClickPos.WorldRay, HitLocation, HitNormal, HitT))
	{
		return FInputRayHit(HitT);
	}
	return FInputRayHit();
}

void UDrawHandyManSpline::OnClicked(const FInputDeviceRay& ClickPos)
{
	using namespace HandyMan::SplineUtils;

	FVector3d HitLocation, HitNormal;
	double HitT;
	if (Raycast(ClickPos.WorldRay, HitLocation, HitNormal, HitT))
	{
		switch (Settings->DrawMode)
		{
		case EDrawSplineDrawMode_HandyMan::ClickAutoTangent:
		case EDrawSplineDrawMode_HandyMan::FreeDraw:
		{
			AddSplinePoint(HitLocation, HitNormal);

			int32 PointIndex = WorkingSpline->GetNumberOfSplinePoints() - 1;
			GetToolManager()->EmitObjectChange(this,
				MakeUnique<FSimplePointInsertionChange>(
					HitLocation, 
					WorkingSpline->GetUpVectorAtSplinePoint(PointIndex, ESplineCoordinateSpace::World)),
				AddPointTransactionName);
			break;
		}
		case EDrawSplineDrawMode_HandyMan::TangentDrag:
		{
			AddSplinePoint(HitLocation, HitNormal);

			int32 PointIndex = WorkingSpline->GetNumberOfSplinePoints() - 1;
			WorkingSpline->SetTangentAtSplinePoint(PointIndex,
				FVector3d::Zero(),
				ESplineCoordinateSpace::World, true);

			GetToolManager()->EmitObjectChange(this,
				MakeUnique<FTangentPointInsertionChange>( 
					HitLocation, 
					WorkingSpline->GetUpVectorAtSplinePoint(PointIndex, ESplineCoordinateSpace::World), 
					FVector3d::Zero()),
				AddPointTransactionName);
			break;
		}
		}

		bNeedToRerunConstructionScript = true;
	}
}

FInputRayHit UDrawHandyManSpline::CanBeginClickDragSequence(const FInputDeviceRay& PressPos)
{
	FVector3d HitLocation, HitNormal;
	double HitT;
	if (Raycast(PressPos.WorldRay, HitLocation, HitNormal, HitT))
	{
		return FInputRayHit(HitT);
	}
	return FInputRayHit();
}

void UDrawHandyManSpline::OnClickPress(const FInputDeviceRay& PressPos)
{
	FVector3d HitLocation, HitNormal;
	double HitT;

	LongTransactions.Open(HandyMan::SplineUtils::AddPointTransactionName, GetToolManager());

	// Regardless of DrawMode, start by placing a point, though don't emit a transaction until mouse up
	if (ensure(Raycast(PressPos.WorldRay, HitLocation, HitNormal, HitT)))
	{
		AddSplinePoint(HitLocation, HitNormal);

		if (Settings->DrawMode == EDrawSplineDrawMode_HandyMan::FreeDraw)
		{
			// Remember which point started this stroke
			FreeDrawStrokeStartIndex = WorkingSpline->GetNumberOfSplinePoints() - 1;
		}
		
		bNeedToRerunConstructionScript = bNeedToRerunConstructionScript || Settings->bRerunConstructionScriptOnDrag;
	}
}

void UDrawHandyManSpline::OnClickDrag(const FInputDeviceRay& DragPos)
{
	using namespace HandyMan::SplineUtils;

	int32 NumSplinePoints = WorkingSpline->GetNumberOfSplinePoints();
	if (!ensure(NumSplinePoints > 0))
	{
		return;
	}

	FVector3d HitLocation, HitNormal;
	double HitT;
	if (Raycast(DragPos.WorldRay, HitLocation, HitNormal, HitT))
	{
		switch (Settings->DrawMode)
		{
		case EDrawSplineDrawMode_HandyMan::ClickAutoTangent:
		{
			// Drag the last placed point
			FVector3d UpVector = GetUpVectorToUse(HitLocation, HitNormal, NumSplinePoints);
			WorkingSpline->SetLocationAtSplinePoint(NumSplinePoints - 1, HitLocation, ESplineCoordinateSpace::World, false);
			WorkingSpline->SetUpVectorAtSplinePoint(NumSplinePoints - 1, UpVector, ESplineCoordinateSpace::World, true);

			break;
		}
		case EDrawSplineDrawMode_HandyMan::TangentDrag:
		{
			// Set the tangent
			FVector3d LastPoint = WorkingSpline->GetLocationAtSplinePoint(NumSplinePoints - 1, ESplineCoordinateSpace::World);
			FVector3d Tangent = (HitLocation - LastPoint) / GetTangentScale();
			WorkingSpline->SetTangentAtSplinePoint(NumSplinePoints - 1, Tangent, ESplineCoordinateSpace::World, true);
			bDrawTangentForLastPoint = true;
			break;
		}
			
		case EDrawSplineDrawMode_HandyMan::FreeDraw:
		{
			// Instead of dragging the first placed point (which gets placed in OnClickPress), we drag a second "preview" one
			// until we get far enough from the previous to where we want to place a new control point.
			if (!bFreeDrawPlacedPreviewPoint)
			{
				AddSplinePoint(HitLocation, HitNormal);
				bFreeDrawPlacedPreviewPoint = true;
			}
			else
			{
				FVector3d PreviousPoint = WorkingSpline->GetLocationAtSplinePoint(NumSplinePoints - 2, ESplineCoordinateSpace::World);
				if (FVector3d::DistSquared(HitLocation, PreviousPoint) >= Settings->MinPointSpacing * Settings->MinPointSpacing)
				{
					AddSplinePoint(HitLocation, HitNormal);
				}
				else
				{
					// Drag the preview point
					FVector3d UpVector = GetUpVectorToUse(HitLocation, HitNormal, NumSplinePoints);
					WorkingSpline->SetLocationAtSplinePoint(NumSplinePoints - 1, HitLocation, ESplineCoordinateSpace::World, false);
					WorkingSpline->SetUpVectorAtSplinePoint(NumSplinePoints - 1, UpVector, ESplineCoordinateSpace::World, true);
				}
			}
			break;
		}
		}
	}

	bNeedToRerunConstructionScript = bNeedToRerunConstructionScript || Settings->bRerunConstructionScriptOnDrag;
}

void UDrawHandyManSpline::OnClickRelease(const FInputDeviceRay& ReleasePos)
{
	OnClickDrag(ReleasePos);
	OnTerminateDragSequence();
}
void UDrawHandyManSpline::OnTerminateDragSequence()
{
	using namespace HandyMan::SplineUtils;

	bDrawTangentForLastPoint = false;
	bFreeDrawPlacedPreviewPoint = false;
	bNeedToRerunConstructionScript = true;

	int32 NumSplinePoints = WorkingSpline->GetNumberOfSplinePoints();

	// Emit the appropriate undo transaction
	switch (Settings->DrawMode)
	{
	case EDrawSplineDrawMode_HandyMan::ClickAutoTangent:
	{
		WorkingSpline->GetLocationAtSplinePoint(NumSplinePoints-1, ESplineCoordinateSpace::World);
		WorkingSpline->GetUpVectorAtSplinePoint(NumSplinePoints - 1, ESplineCoordinateSpace::World);
		GetToolManager()->EmitObjectChange(this,
			MakeUnique<FSimplePointInsertionChange>(
				WorkingSpline->GetLocationAtSplinePoint(NumSplinePoints - 1, ESplineCoordinateSpace::World),
				WorkingSpline->GetUpVectorAtSplinePoint(NumSplinePoints - 1, ESplineCoordinateSpace::World)),
			AddPointTransactionName);
		break;
	}
	case EDrawSplineDrawMode_HandyMan::TangentDrag:
	{
		GetToolManager()->EmitObjectChange(this,
			MakeUnique<FTangentPointInsertionChange>( 
				WorkingSpline->GetLocationAtSplinePoint(NumSplinePoints - 1, ESplineCoordinateSpace::World),
				WorkingSpline->GetUpVectorAtSplinePoint(NumSplinePoints - 1, ESplineCoordinateSpace::World),
				WorkingSpline->GetTangentAtSplinePoint(NumSplinePoints - 1, ESplineCoordinateSpace::World)),
			AddPointTransactionName);
		break;
	}

	case EDrawSplineDrawMode_HandyMan::FreeDraw:
	{
		TArray<FVector3d> HitLocations;
		TArray<FVector3d> UpVectors;
		for (int32 i = FreeDrawStrokeStartIndex; i < NumSplinePoints; ++i)
		{
			HitLocations.Add(WorkingSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World));
			UpVectors.Add(WorkingSpline->GetUpVectorAtSplinePoint(i, ESplineCoordinateSpace::World));
		}

		GetToolManager()->EmitObjectChange(this,
			MakeUnique<FStrokeInsertionChange>(HitLocations, UpVectors),
			AddPointTransactionName);
		break;
	}
	}

	LongTransactions.Close(GetToolManager());
}


void UDrawHandyManSpline::OnTick(float DeltaTime)
{
	if (PlaneMechanic)
	{
		PlaneMechanic->Tick(DeltaTime);
	}

	// check if we've invalidated the WorkingSpline
	if (PreviewActor && !WorkingSpline.IsValid())
	{
		bNeedToRerunConstructionScript = true;
	}

	if (bNeedToRerunConstructionScript)
	{
		bNeedToRerunConstructionScript = false;
		if (PreviewActor)
		{
			PreviewActor->RerunConstructionScripts();

			// Rerunning the construction script can make us lose our reference to the spline, so try to
			// recapture.
			// TODO: This might be avoidable with FComponentReference?
			if (!WorkingSpline.IsValid())
			{
				TInlineComponentArray<USplineComponent*> SplineComponents;
				PreviewActor->GetComponents<USplineComponent>(SplineComponents);

				if (ensure(SplineRecaptureIndex >= 0 && SplineRecaptureIndex < SplineComponents.Num()))
				{
					WorkingSpline = SplineComponents[SplineRecaptureIndex];
				}
				else
				{
					// If we failed to recapture, it's not clear what to do. We can switch to working inside
					// an empty actor, though we'll lose current spline state.
					Settings->OutputMode = EDrawSplineOutputMode_HandyMan::EmptyActor;
					Settings->CheckAndUpdateWatched();
				}
			}
		}
	}

	if (!WorkingSpline.IsValid())
	{
		GetToolManager()->PostActiveToolShutdownRequest(this, EToolShutdownType::Cancel, true, LOCTEXT("LostWorkingSpline", "The Draw Spline tool must close because the in-progress spline has been unexpectedly deleted."));
	}
}

void UDrawHandyManSpline::Render(IToolsContextRenderAPI* RenderAPI)
{
	using namespace HandyMan::SplineUtils;

	Super::Render(RenderAPI);

	GetToolManager()->GetContextQueriesAPI()->GetCurrentViewState(CameraState);

	if (PlaneMechanic)
	{
		PlaneMechanic->Render(RenderAPI);
	}

	if (WorkingSpline.IsValid())
	{
		if (bDrawTangentForLastPoint)
		{
			DrawTangent(*WorkingSpline, WorkingSpline->GetNumberOfSplinePoints() - 1, *RenderAPI);
		}

		SplineUtil::FDrawSplineSettings DrawSettings;
		DrawSettings.ScaleVisualizationWidth = Settings->FrameVisualizationWidth;
		SplineUtil::DrawSpline(*WorkingSpline, *RenderAPI, DrawSettings);
	}
}

void UDrawHandyManSpline::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{
}

bool UDrawHandyManSpline::CanAccept() const
{
	return WorkingSpline.IsValid() && WorkingSpline->GetNumberOfSplinePoints() > 0;
}


UBaseScriptableToolBuilder* UDrawHandyManSpline::GetNewCustomToolBuilderInstance(UObject* Outer)
{
	return Cast<UBaseScriptableToolBuilder>( NewObject<UDrawHandyManSplineBuilder>(Outer));
}

// To be called by builder
void UDrawHandyManSpline::SetSelectedActor(AActor* Actor)
{
	SelectedActor = Actor;
}

/// Tool builder:

bool UDrawHandyManSplineBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return true;
}

UInteractiveTool* UDrawHandyManSplineBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UDrawHandyManSpline* NewTool = NewObject<UDrawHandyManSpline>(SceneState.ToolManager);
	NewTool->SetTargetWorld(SceneState.World);

	// May be null
	NewTool->SetSelectedActor(ToolBuilderUtil::FindFirstActor(SceneState, [](AActor*) { return true; }));

	return NewTool;
}


void UDrawHandyManSpline::FDrawHandyManSplineToolChange::Apply(UObject* Object)
{
	UDrawHandyManSpline* Tool = Cast<UDrawHandyManSpline>(Object);
	if (!ensure(Tool))
	{
		return;
	}
	TWeakObjectPtr<USplineComponent> Spline = Tool->WorkingSpline;
	if (!ensure(Spline.IsValid()))
	{
		return;
	}

	Apply(*Spline);

	Tool->bNeedToRerunConstructionScript = true;
}

void UDrawHandyManSpline::FDrawHandyManSplineToolChange::Revert(UObject* Object)
{
	UDrawHandyManSpline* Tool = Cast<UDrawHandyManSpline>(Object);
	if (!ensure(Tool))
	{
		return;
	}
	TWeakObjectPtr<USplineComponent> Spline = Tool->WorkingSpline;
	if (!ensure(Spline.IsValid()))
	{
		return;
	}

	Revert(*Spline);

	Tool->bNeedToRerunConstructionScript = true;
}

#undef LOCTEXT_NAMESPACE
