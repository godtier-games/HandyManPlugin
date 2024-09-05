// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicBasedScatterTool.h"

#include "DynamicMeshEditor.h"
#include "IMeshMergeUtilities.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "InteractiveToolManager.h"
#include "LevelEditorSubsystem.h"
#include "MeshMergeModule.h"
#include "ModelingObjectsCreationAPI.h"
#include "ModelingToolTargetUtil.h"
#include "PBDRigidsSolver.h"
#include "Selection.h"
#include "ToolTargetManager.h"
#include "UnrealEdGlobals.h"
#include "BaseBehaviors/MouseHoverBehavior.h"
#include "ConversionUtils/SceneComponentToDynamicMesh.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/StaticMeshActor.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshModelingFunctions.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MeshMerge/MeshInstancingSettings.h"
#include "Physics/ComponentCollisionUtil.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PropertySets/OnAcceptProperties.h"
#include "Selection/ToolSelectionUtil.h"
#include "TargetInterfaces/DynamicMeshCommitter.h"
#include "TargetInterfaces/DynamicMeshProvider.h"
#include "TargetInterfaces/MeshTargetInterfaceTypes.h"
#include "TargetInterfaces/PrimitiveComponentBackedTarget.h"

#define LOCTEXT_NAMESPACE "PhysicBasedScatterTool"

namespace PhysicsBasedLocals
{
	void SetNewMaterialID(int32 ComponentIdx, UE::Geometry::FDynamicMeshMaterialAttribute* MatAttrib, int32 TID, 
		TArray<TArray<int32>>& MaterialIDRemaps, TArray<UMaterialInterface*>& AllMaterials)
	{
		int MatID = MatAttrib->GetValue(TID);
		if (!ensure(MatID >= 0))
		{
			return;
		}
		if (MatID >= MaterialIDRemaps[ComponentIdx].Num())
		{
			UE_LOG(LogGeometry, Warning, TEXT("UCombineMeshesTool: Component %d had at least one material ID (%d) "
				"that was not in its material list."), ComponentIdx, MatID);
			
			// There are different things we could do here. It's worth noting that out of bounds material indices
			// are handled differently in static meshes and dynamic mesh components, and depend in part on how we
			// got to that state. So trying to preserve a specific behavior is not practical, and probably not 
			// expected if the user is not giving us valid data to begin with.
			// The route we go is to give a separate nullptr material slot to each out of bounds ID. This will give
			// the user a chance to preserve their material assignments while fixing the issue by assigning materials to
			// the slots created in the output (at least, unless they pass through this tool again, at which point any
			// nullptr-pointing IDs will be collapsed to point to the same nullptr slot, due to the way we create the
			// combined material list for in-bounds IDs).
			int32 NumElementsToAdd = MatID - MaterialIDRemaps[ComponentIdx].Num() + 1;
			for (int32 i = 0; i < NumElementsToAdd; ++i)
			{
				MaterialIDRemaps[ComponentIdx].Add(AllMaterials.Num());
				AllMaterials.Add(nullptr);
			}
			checkSlow(MaterialIDRemaps[ComponentIdx].Num() == MatID + 1);
		}
		MatAttrib->SetValue(TID, MaterialIDRemaps[ComponentIdx][MatID]);
	}
}



UPhysicBasedScatterTool::UPhysicBasedScatterTool()
{
	ToolName = LOCTEXT("PhysicBasedScatterToolName", "Dropper Tool");
	ToolTooltip = LOCTEXT("PhysicBasedScatterToolDescription", "Scatter objects based on physics");
	ToolLongName = LOCTEXT("PhysicBasedScatterToolLongName", "Physics Dropper Tool");
	ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
	bWantMouseHover = true;
	bUpdateModifiersDuringDrag = true;
}

void UPhysicBasedScatterTool::CreateBrush()
{
	UMaterial* BrushMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EditorLandscapeResources/FoliageBrushSphereMaterial.FoliageBrushSphereMaterial"), nullptr, LOAD_None, nullptr);
	BrushMI = UMaterialInstanceDynamic::Create(BrushMaterial, GetTransientPackage());
	BrushMI->SetVectorParameterValue(TEXT("HighlightColor"), FLinearColor::Blue);
	check(BrushMI != nullptr);

	Brush = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("SphereBrushComponent"));
	Brush->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Brush->SetCollisionObjectType(ECC_WorldDynamic);
	Brush->SetMaterial(0, BrushMI);
	Brush->SetAbsolute(true, true, true);
	Brush->CastShadow = false;
}

void UPhysicBasedScatterTool::OnLevelActorsAdded(AActor* InActor)
{
	if (InActor && InActor->IsA<AStaticMeshActor>() && !InActor->IsActorBeingDestroyed())
	{
		if (InActor->GetLevel() == GetWorld()->GetCurrentLevel())
		{
			if (!SpawnedActors.Contains(InActor))
			{
				UpdatePhysics(InActor, PropertySet->IsEnableGravity());
				SpawnedActors.Add(InActor);

			}
		}
	}
}

void UPhysicBasedScatterTool::OnLevelActorsDeleted(AActor* InActor)
{
	if (SpawnedActors.Contains(InActor))
	{
		SpawnedActors.Remove(InActor);
	}

	if (LastSpawnedActors.Contains(InActor))
	{
		LastSpawnedActors.Remove(InActor);
	}

	if (LastSelectedActors.Contains(InActor))
	{
		LastSelectedActors.Remove(InActor);
	}
	if (LevelActors.Contains(InActor))
	{
		LevelActors.Remove(InActor);
	}

	auto Prims = GetPrimitives(InActor);
	for (auto& Prim : Prims)
	{
		if (Mobilities.Contains(Prim))
		{
			Mobilities.Remove(Prim);
		}
		if (Physics.Contains(Prim))
		{
			Physics.Remove(Prim);
		}
		if (Gravities.Contains(Prim))
		{
			Gravities.Remove(Prim);
		}
		if (Positions.Contains(Prim))
		{
			Positions.Remove(Prim);
		}
		if (Rotations.Contains(Prim))
		{
			Rotations.Remove(Prim);
		}
	}
	// Physics
	// Gravities
	// Positions
	// Rotations
}

void UPhysicBasedScatterTool::Setup()
{
	Super::Setup();
	
	OnLevelActorsAddedHandle = GEngine->OnLevelActorAdded().AddUObject(this, &ThisClass::OnLevelActorsAdded);
	OnLevelActorsDeletedHandle = GEngine->OnLevelActorDeleted().AddUObject(this, &ThisClass::OnLevelActorsDeleted);
	OnPreBeginPieHandle = FEditorDelegates::PreBeginPIE.AddUObject(this, &ThisClass::OnPreBeginPie);
	

	CreateBrush();

	EToolsFrameworkOutcomePins PropertyCreationOutcome;
	PropertySet = Cast<UPhysicsDropPropertySet>(AddPropertySetOfType(UPhysicsDropPropertySet::StaticClass(), "Settings", PropertyCreationOutcome));

	PropertySet->SetRandomMesh();

}

bool UPhysicBasedScatterTool::OnUpdateHover(const FInputDeviceRay& DevicePos)
{
	FHitResult Hit;
	const bool bWasHit = Trace(Hit, DevicePos);

	auto Rotation = PropertySet->GetRotateRandom();
	auto PickedMesh = PropertySet->GetRandomMesh();
	
	if (PickedMesh == nullptr)
	{
		PropertySet->SetRandomMesh();
		PickedMesh = PropertySet->GetRandomMesh();
	}

	if (IsValid(PickedMesh))
	{
		if (IsValid(Brush))
		{
			if (!Brush->IsRegistered())
			{
				Brush->RegisterComponentWithWorld(GetWorld());
			}

			Brush->SetStaticMesh(PickedMesh);
			Brush->SetWorldLocation(GetPosition());
			Brush->SetWorldScale3D(PropertySet->GetScaleRandom());

			if (IsShiftDown() && IsCtrlDown())
			{
				Rotation = UKismetMathLibrary::FindLookAtRotation(
					BrushPosition,
					BrushPosition + BrushDirection) + PropertySet->GetNormalRotation();
			}
			else if (IsShiftDown())
			{
				Rotation = UKismetMathLibrary::FindLookAtRotation(
					BrushPosition,
					BrushPosition + BrushNormal) + PropertySet->GetNormalRotation();
			}

			Brush->SetWorldRotation(Rotation);
		}
		else
		{
			Brush->SetVisibility(false);
		}
	}
	

	return bWasHit;
}

void UPhysicBasedScatterTool::OnClickDrag(const FInputDeviceRay& DragPos)
{
	Super::OnClickDrag(DragPos);
	
	FHitResult Hit;
	const bool bWasHit = Trace(Hit, DragPos);

	LatestPosition = Hit.Location;

	auto Rotation = PropertySet->GetRotateRandom();
	auto PickedMesh = PropertySet->GetRandomMesh();
	
	if (PickedMesh == nullptr)
	{
		PropertySet->SetRandomMesh();
		PickedMesh = PropertySet->GetRandomMesh();
	}
	
	if (bWasHit && IsValid(PickedMesh))
	{
		// Spawn Actors In A Grid Above the hit locatio
		if (IsCtrlDown() && !IsShiftDown() && !IsAltDown())
		{
			if (SpawnedActors.Contains(Hit.GetActor()))
			{
				GetWorld()->DestroyActor(Hit.GetActor());
			}
		}
		else
		{
			auto ReferenceMesh = PropertySet->GetRandomMesh();
			if (IsValid(ReferenceMesh))
			{
				float dist = FVector::Dist(GetPosition(), LastSpawnedPosition);
				if (dist > PropertySet->MinDistance)
				{
					LastSpawnedPosition = GetPosition();

					FActorSpawnParameters Params = FActorSpawnParameters();
					FString name = FString::Format(TEXT("Actor_{0}"), { ReferenceMesh->GetFName().ToString() });
					FName fname = MakeUniqueObjectName(nullptr, AStaticMeshActor::StaticClass(), FName(*name));
					Params.Name = fname;
					Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;

					AStaticMeshActor* actor = GetWorld()->SpawnActor<AStaticMeshActor>(GetPosition(), Rotation, Params);

					actor->SetActorLabel(fname.ToString());
					actor->SetActorScale3D(PropertySet->GetScaleRandom());
					LastSpawnedActors.Add(actor);
					actor->GetStaticMeshComponent()->SetStaticMesh(ReferenceMesh);

					UpdatePhysics(actor, PropertySet->IsEnableGravity());
					PropertySet->SetRandomMesh();
				}
			}
		}

		if (IsValid(Brush))
		{
			if (!Brush->IsRegistered())
			{
				Brush->RegisterComponentWithWorld(GetWorld());
			}
			Brush->SetStaticMesh(PropertySet->GetRandomMesh());
		}

	}
}

bool UPhysicBasedScatterTool::Trace(FHitResult& OutHit, const FInputDeviceRay& DevicePos)
{
	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
	Params.AddIgnoredActors(LastSpawnedActors);

	bool bBeenHit = GetWorld()->LineTraceSingleByChannel(
		OutHit, 
		DevicePos.WorldRay.Origin, 
		DevicePos.WorldRay.Origin + DevicePos.WorldRay.Direction * HALF_WORLD_MAX, 
		ECollisionChannel::ECC_Visibility, Params);

	if (bBeenHit)
	{
		BrushLastPosition = BrushPosition;
		BrushPosition = OutHit.ImpactPoint;
		BrushDirection = BrushLastPosition - BrushPosition;
		BrushNormal = OutHit.ImpactNormal;
	}

	return bBeenHit;

}

void UPhysicBasedScatterTool::OnPreBeginPie(bool InStarted)
{
}


void UPhysicBasedScatterTool::OnPressedFunc(FInputDeviceRay ClickPos,FScriptableToolModifierStates Modifiers, EScriptableToolMouseButton MouseButton)
{
	switch (MouseButton)
	{
	case EScriptableToolMouseButton::LeftButton:
		bCanSpawn = true;
		break;
	case EScriptableToolMouseButton::RightButton:
		break;
	case EScriptableToolMouseButton::MiddleButton:
		break;
	}
}

void UPhysicBasedScatterTool::OnReleaseFunc(FInputDeviceRay ClickPos, FScriptableToolModifierStates Modifiers, EScriptableToolMouseButton MouseButton)
{
	switch (MouseButton)
	{
	case EScriptableToolMouseButton::LeftButton:
		bCanSpawn = false;
		break;
	case EScriptableToolMouseButton::RightButton:
		break;
	case EScriptableToolMouseButton::MiddleButton:
		break;
	}
}

bool UPhysicBasedScatterTool::MouseBehaviorModiferCheckFunc(const FInputDeviceState& InputDeviceState)
{
	
	return bCanSpawn;
}

void UPhysicBasedScatterTool::ActorSelectionChangeNotify()
{
	if (IsValid(PropertySet))
	{
		if (PropertySet->IsSelectingPlacedActors())
		{
			TArray<AActor*> Actors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
			for (auto& Actor : Actors)
			{
				if (!SpawnedActors.Contains(Actor))
				{
					GEditor->SelectActor(Actor, false, false);
				}
			}
		}
	}

	TArray<AActor*> Actors = GetSelectedActors();
	bool SelectionChanged = false;
	for (auto& Actor : Actors)
	{
		if (!LastSelectedActors.Contains(Actor))
		{
			SelectionChanged = true;
			break;
		}
	}
	
	if (SelectionChanged)
	{
		LastSelectedActors = GetSelectedActors();
		UpdateSelectionPhysics();
	}
}


void UPhysicBasedScatterTool::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);

	if (GEditor->IsSimulateInEditorInProgress() || GEditor->IsPlaySessionInProgress()
		|| GEditor->IsPlaySessionRequestQueued())
	{
		return;
	}
	
	if (IsValid(PropertySet))
	{
		/*FHitResult Hit;
		bool Hited = Trace(Hit, ViewportClient);

		if (GetCurrentLayoutMode() == ELayoutMode::PaintSelect)
		{
			if (Hited && bIsPainting)
			{
				if (PropertySet->IsSelectingPlacedActors())
				{
					if (SpawnedActors.Contains(Hit.GetActor()))
					{
						GEditor->SelectActor(Hit.GetActor(), !bIsCtrlDown, true);
					}
				}
				else
				{
					GEditor->SelectActor(Hit.GetActor(), !bIsCtrlDown, true);
				}
			}
		}
		else if (GetCurrentLayoutMode() == ELayoutMode::Paint)
		{
			if (Hited)
			{
				auto Rotation = PropertySet->GetRotateRandom();
				auto PickedMesh = PropertySet->GetRandomMesh();

				if (IsValid(PickedMesh))
				{
					if (IsValid(Brush))
					{
						if (!Brush->IsRegistered())
						{
							Brush->RegisterComponentWithWorld(GetWorld());
						}

						Brush->SetStaticMesh(PickedMesh);
						Brush->SetWorldLocation(GetPosition());
						Brush->SetWorldScale3D(PropertySet->GetScaleRandom());

						if (bIsShiftDown && bIsCtrlDown)
						{
							Rotation = UKismetMathLibrary::FindLookAtRotation(
								BrushPosition,
								BrushPosition + BrushDirection) + PropertySet->GetNormalRotation();
						}
						else if (bIsShiftDown)
						{
							Rotation = UKismetMathLibrary::FindLookAtRotation(
								BrushPosition,
								BrushPosition + BrushNormal) + PropertySet->GetNormalRotation();
						}

						Brush->SetWorldRotation(Rotation);
					}
					else
					{
						Brush->SetVisibility(false);
					}
					
				}
			}
		}*/
		auto World = GetWorld();
		auto Solver = World->GetPhysicsScene()->GetSolver();

		if (PropertySet->GetLayoutMode() == ELayoutMode::Transform ||
			PropertySet->GetLayoutMode() == ELayoutMode::Paint)
		{
			TArray<AActor*> Actors = GetSelectedActors();
			Solver->StartingSceneSimulation();
			if (Actors.Num() > 0)
			{
				for (int i = 0; i < Actors.Num(); ++i)
				{
					AActor* SelectedActor =  Actors[i];
					if (SelectedActor)
					{
						bool DampVelocity = PropertySet->IsDamplingVelocity();
						if (!PropertySet->IsDamplingVelocity())
						{
							//DampVelocity = GetCurrentWidgetAxis() != EAxisList::None && GetWidgetLocation() == SelectedActor->GetActorLocation();
						}
						if (DampVelocity)
						{
							TArray<UPrimitiveComponent*> Prims = GetPrimitives(SelectedActor);
							for (auto& Prim : Prims)
							{
								Prim->SetPhysicsLinearVelocity(FVector::ZeroVector);
								Prim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
							}
						}
					}
				}

				UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), LevelActors);
				if (LevelActors.Num() > 0)
				{
					for (int i = 0; i < LevelActors.Num(); ++i)
					{
						AActor* LevelActor = LevelActors[i];
						if (LevelActor)
						{
							if (AGeometryCollectionActor* CollectionActor = Cast<AGeometryCollectionActor>(LevelActor))
							{
								auto CollectionComponent = CollectionActor->GetGeometryCollectionComponent();
							}
							else
							{

								bool DampVelocity = PropertySet->IsDamplingVelocity();
								if (!PropertySet->IsDamplingVelocity())
								{
									// TODO : I think this has something to do with checking if the gizmo and the actor are in the same location
									//DampVelocity = GetCurrentWidgetAxis() != EAxisList::None && GetWidgetLocation() == LevelActor->GetActorLocation();
								}

								if (DampVelocity)
								{
									auto Prims = GetPrimitives(LevelActor);
									for (auto& Prim : Prims)
									{
										Prim->SetPhysicsLinearVelocity(FVector::ZeroVector);
										Prim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
									}
								}
							}
						}
					}
				}
			}
			Solver->AdvanceAndDispatch_External(DeltaTime);
		}

		if (GetCurrentLayoutMode() != ELayoutMode::Transform)
		{
			DrawDebugLine(GetWorld(), BrushPosition, (BrushPosition + BrushDirection), BrushColor.ToFColor(false), false, -1, 0, 5);
		}
	}
}

void UPhysicBasedScatterTool::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);

	GEngine->OnLevelActorAdded().Remove(OnLevelActorsAddedHandle);
	GEngine->OnLevelActorDeleted().Remove(OnLevelActorsDeletedHandle);
	FEditorDelegates::PreBeginPIE.Remove(OnPreBeginPieHandle);
	
	GetWorld()->FinishPhysicsSim();

	TArray<UPrimitiveComponent*> SpawnedComponents = GetSpawnedComponents();
	
	// Set the actors as targets
	FToolBuilderState SceneState;
	GetToolManager()->GetContextQueriesAPI()->GetCurrentSelectionState(SceneState);
	for (const auto Item : SpawnedComponents)
	{
		SceneState.SelectedComponents.Add(Item);
	}
	static FToolTargetTypeRequirements TypeRequirements({
		UStreamableRenderAsset::StaticClass(),
	});
	TArray<TObjectPtr<UToolTarget>> NewTargets = SceneState.TargetManager->BuildAllSelectedTargetable(SceneState, FToolTargetTypeRequirements());
	if(NewTargets.Num() < 2)
	{
		return;
	}
	SetTargets(NewTargets);

	
	
	switch (ShutdownType)
	{
	case EToolShutdownType::Completed:
		break;
	case EToolShutdownType::Accept:
		HandleAccept();
		break;
	case EToolShutdownType::Cancel:
		HandleCancel();
		break;
	}

	ResetPhysics();
	Mobilities.Reset();
	Gravities.Reset();
	Physics.Reset();
	LevelActors.Reset();
	LastSpawnedActors.Reset();
	SpawnedActors.Reset();
	Brush->UnregisterComponent();
	Brush->SetStaticMesh(nullptr);
}

void UPhysicBasedScatterTool::UpdateAcceptWarnings(EAcceptWarning Warning)
{
	Super::UpdateAcceptWarnings(Warning);
}

bool UPhysicBasedScatterTool::CanAccept() const
{
	return Super::CanAccept() && SpawnedActors.Num() > 0;
}

void UPhysicBasedScatterTool::HandleAccept()
{
	auto ReturnType = FMessageDialog::Open(EAppMsgCategory::Warning, EAppMsgType::YesNo, LOCTEXT("DrawSplineNoInputGeometry", "Bake Physics Actors?"));

	switch (ReturnType) {
	case EAppReturnType::No:
		break;
	case EAppReturnType::Yes:
		CreateNewAsset();
		break;
	case EAppReturnType::YesAll:
		break;
	case EAppReturnType::NoAll:
		break;
	case EAppReturnType::Cancel:
		break;
	case EAppReturnType::Ok:
		break;
	case EAppReturnType::Retry:
		break;
	case EAppReturnType::Continue:
		break;
	}
}

void UPhysicBasedScatterTool::BakeToInstanceMesh(bool BakeSelected)
{
	FScopedSlowTask SlowTask(0, LOCTEXT("UPhysicBasedScatterToolSlowTask", "Merging actors..."));
	SlowTask.MakeDialog();

	GetToolManager()->BeginUndoTransaction(LOCTEXT("UPhysicBasedScatterTool", "Convert Meshes"));
	
	TArray<UPrimitiveComponent*> SpawnedComponents;
	if (BakeSelected)
	{
		SpawnedComponents = GetSelectedPrimitives();
	}
	else
	{
		SpawnedComponents = GetSpawnedComponents();
	}
	
	if (SpawnedComponents.Num() > 0)
	{
		GEditor->BeginTransaction(LOCTEXT("PhysicalLayoutMode_Bake", "Bake to InstanceMesh"));
		FMeshInstancingSettings Settings;
		const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();
		MeshUtilities.MergeComponentsToInstances(SpawnedComponents, GetWorld(), GetWorld()->GetCurrentLevel(), Settings);
		GEditor->EndTransaction();
	}

	GetToolManager()->EndUndoTransaction();
		
	
	
	DestroyActors(BakeSelected);
}

void UPhysicBasedScatterTool::CreateNewAsset()
{
	using namespace PhysicsBasedLocals;

	// Make sure meshes are available before we open transaction. This is to avoid potential stability issues related 
	// to creation/load of meshes inside a transaction, for assets that possibly do not have bulk data currently loaded.
	static FGetMeshParameters GetMeshParams;
	GetMeshParams.bWantMeshTangents = true;
	TArray<UE::Geometry::FDynamicMesh3> InputMeshes;
	InputMeshes.Reserve(Targets.Num());
	for (int32 ComponentIdx = 0; ComponentIdx < Targets.Num(); ComponentIdx++)
	{
		InputMeshes.Add(UE::ToolTarget::GetDynamicMeshCopy(Targets[ComponentIdx], GetMeshParams));
	}

	GetToolManager()->BeginUndoTransaction(
		LOCTEXT("DuplicateMeshToolTransactionName", "Duplicate Mesh"));

	FBox Box(ForceInit);
	for (int32 ComponentIdx = 0; ComponentIdx < Targets.Num(); ComponentIdx++)
	{
		Box += UE::ToolTarget::GetTargetComponent(Targets[ComponentIdx])->Bounds.GetBox();
	}

	TArray<UMaterialInterface*> AllMaterials;
	TArray<TArray<int32>> MaterialIDRemaps;
	BuildCombinedMaterialSet(AllMaterials, MaterialIDRemaps);

	UE::Geometry::FDynamicMesh3 AccumulateDMesh;
	AccumulateDMesh.EnableTriangleGroups();
	AccumulateDMesh.EnableAttributes();
	AccumulateDMesh.Attributes()->EnableTangents();
	AccumulateDMesh.Attributes()->EnableMaterialID();
	AccumulateDMesh.Attributes()->EnablePrimaryColors();
	constexpr bool bCenterPivot = false;
	FVector3d Origin = FVector3d::ZeroVector;
	if (bCenterPivot)
	{
		// Place the pivot at the bounding box center
		Origin = Box.GetCenter();
	}
	else if (!Targets.IsEmpty())
	{
		// Use the average pivot
		for (int32 ComponentIdx = 0; ComponentIdx < Targets.Num(); ComponentIdx++)
		{
			Origin += UE::ToolTarget::GetLocalToWorldTransform(Targets[ComponentIdx]).TransformPosition(FVector3d::ZeroVector);
		}
		Origin /= Targets.Num();
	}
	FTransform3d AccumToWorld(Origin);
	FTransform3d ToAccum(-Origin);

	UE::Geometry::FSimpleShapeSet3d SimpleCollision;
	UE::Geometry::FComponentCollisionSettings CollisionSettings;

	{
#if WITH_EDITOR
		FScopedSlowTask SlowTask(Targets.Num()+1, 
			LOCTEXT("DuplicateMeshBuild", "Merging mesh ..."));
		SlowTask.MakeDialog();
#endif
		bool bNeedColorAttr = false;
		int MatIndexBase = 0;
		for (int32 ComponentIdx = 0; ComponentIdx < Targets.Num(); ComponentIdx++)
		{
#if WITH_EDITOR
			SlowTask.EnterProgressFrame(1);
#endif
			UPrimitiveComponent* PrimitiveComponent = UE::ToolTarget::GetTargetComponent(Targets[ComponentIdx]);

			UE::Geometry::FDynamicMesh3& ComponentDMesh = InputMeshes[ComponentIdx];
			bNeedColorAttr = bNeedColorAttr || (ComponentDMesh.HasAttributes() && ComponentDMesh.Attributes()->HasPrimaryColors());

			if (ComponentDMesh.HasAttributes())
			{
				AccumulateDMesh.Attributes()->EnableMatchingAttributes(*ComponentDMesh.Attributes(), false);
			}

			// update material IDs to account for combined material set
			UE::Geometry::FDynamicMeshMaterialAttribute* MatAttrib = ComponentDMesh.Attributes()->GetMaterialID();
			for (int TID : ComponentDMesh.TriangleIndicesItr())
			{
				SetNewMaterialID(ComponentIdx, MatAttrib, TID, MaterialIDRemaps, AllMaterials);
			}

			UE::Geometry::FDynamicMeshEditor Editor(&AccumulateDMesh);
			UE::Geometry::FMeshIndexMappings IndexMapping;
			
			UE::Geometry::FTransformSRT3d XF = (UE::ToolTarget::GetLocalToWorldTransform(Targets[ComponentIdx]) * ToAccum);
			if (XF.GetDeterminant() < 0)
			{
				ComponentDMesh.ReverseOrientation(false);
			}

			Editor.AppendMesh(&ComponentDMesh, IndexMapping,
				[&XF](int Unused, const FVector3d P) { return XF.TransformPosition(P); },
				[&XF](int Unused, const FVector3d N) { return XF.TransformNormal(N); });
			if (UE::Geometry::ComponentTypeSupportsCollision(PrimitiveComponent))
			{
				UE::Geometry::AppendSimpleCollision(PrimitiveComponent, &SimpleCollision, XF);
			}
			
			FComponentMaterialSet MaterialSet = UE::ToolTarget::GetMaterialSet(Targets[ComponentIdx]);
			MatIndexBase += MaterialSet.Materials.Num();
		}

		if (!bNeedColorAttr)
		{
			AccumulateDMesh.Attributes()->DisablePrimaryColors();
		}

#if WITH_EDITOR
		SlowTask.EnterProgressFrame(1);
#endif
		
		// max len explicitly enforced here, would ideally notify user
		FString UseBaseName = UE::ToolTarget::GetTargetActor(Targets[0])->GetName();
		if (UseBaseName.IsEmpty())
		{
			UseBaseName = TEXT("Simulated");
		}
		else
		{
			UseBaseName = TEXT("Simulated_") + UseBaseName;
		}

		FCreateMeshObjectParams NewMeshObjectParams;
		NewMeshObjectParams.TargetWorld = GetWorld();
		NewMeshObjectParams.Transform = (FTransform)AccumToWorld;
		NewMeshObjectParams.BaseName = UseBaseName;
		NewMeshObjectParams.Materials = AllMaterials;
		NewMeshObjectParams.SetMesh(&AccumulateDMesh);

		NewMeshObjectParams.TypeHint = ECreateObjectTypeHint::StaticMesh;
		
		FCreateMeshObjectResult Result = UE::Modeling::CreateMeshObject(GetToolManager(), MoveTemp(NewMeshObjectParams));
		if (Result.IsOK() && Result.NewActor != nullptr)
		{
			// if any inputs have Simple Collision geometry we will forward it to new mesh.
			if (UE::Geometry::ComponentTypeSupportsCollision(Result.NewComponent) && SimpleCollision.TotalElementsNum() > 0)
			{
				UE::Geometry::SetSimpleCollision(Result.NewComponent, &SimpleCollision, CollisionSettings);
			}

			// select the new actor
			ToolSelectionUtil::SetNewActorSelection(GetToolManager(), Result.NewActor);
		}
	}
	
	TArray<AActor*> Actors;
	for (int32 Idx = 0; Idx < Targets.Num(); Idx++)
	{
		Actors.Add(UE::ToolTarget::GetTargetActor(Targets[Idx]));
	}
	
	ApplyMethod(Actors, GetToolManager());


	GetToolManager()->EndUndoTransaction();
}

void UPhysicBasedScatterTool::BuildCombinedMaterialSet(TArray<UMaterialInterface*>& NewMaterialsOut,TArray<TArray<int32>>& MaterialIDRemapsOut)
{
	NewMaterialsOut.Reset();

	TMap<UMaterialInterface*, int> KnownMaterials;

	MaterialIDRemapsOut.SetNum(Targets.Num());
	for (int32 ComponentIdx = 0; ComponentIdx < Targets.Num(); ComponentIdx++)
	{
		FComponentMaterialSet MaterialSet = UE::ToolTarget::GetMaterialSet(Targets[ComponentIdx]);
		int32 NumMaterials = MaterialSet.Materials.Num();
		for (int MaterialIdx = 0; MaterialIdx < NumMaterials; MaterialIdx++)
		{
			UMaterialInterface* Mat = MaterialSet.Materials[MaterialIdx];
			int32 NewMaterialIdx = 0;
			if (KnownMaterials.Contains(Mat) == false)
			{
				NewMaterialIdx = NewMaterialsOut.Num();
				KnownMaterials.Add(Mat, NewMaterialIdx);
				NewMaterialsOut.Add(Mat);
			}
			else
			{
				NewMaterialIdx = KnownMaterials[Mat];
			}
			MaterialIDRemapsOut[ComponentIdx].Add(NewMaterialIdx);
		}
	}
}

void UPhysicBasedScatterTool::ApplyMethod(const TArray<AActor*>& Actors, UInteractiveToolManager* ToolManager, const AActor* MustKeepActor)
{
	const EHandleSourcesMethod HandleInputs = EHandleSourcesMethod::DeleteSources;

	// Hide or destroy the sources
	bool bKeepSources = HandleInputs == EHandleSourcesMethod::KeepSources;
	if (Actors.Num() == 1 && (HandleInputs == EHandleSourcesMethod::KeepFirstSource || HandleInputs == EHandleSourcesMethod::KeepLastSource))
	{
		// if there's only one actor, keeping any source == keeping all sources
		bKeepSources = true;
	}
	if (!bKeepSources)
	{
		bool bDelete = HandleInputs == EHandleSourcesMethod::DeleteSources
					|| HandleInputs == EHandleSourcesMethod::KeepFirstSource
					|| HandleInputs == EHandleSourcesMethod::KeepLastSource;
		if (bDelete)
		{
			ToolManager->BeginUndoTransaction(LOCTEXT("RemoveSources", "Remove Inputs"));
		}
		else
		{
#if WITH_EDITOR
			ToolManager->BeginUndoTransaction(LOCTEXT("HideSources", "Hide Inputs"));
#endif
		}

		const int32 ActorIdxBegin = HandleInputs == EHandleSourcesMethod::KeepFirstSource ? 1 : 0;
		const int32 ActorIdxEnd = HandleInputs == EHandleSourcesMethod::KeepLastSource ? Actors.Num() - 1 : Actors.Num();

		for (int32 ActorIdx = ActorIdxBegin; ActorIdx < ActorIdxEnd; ActorIdx++)
		{
			AActor* Actor = Actors[ActorIdx];
			if (Actor == MustKeepActor)
			{
				continue;
			}

			if (bDelete)
			{
				if (UWorld* ActorWorld = Actor->GetWorld())
				{
#if WITH_EDITOR
					if (GIsEditor && GUnrealEd)
					{
						GUnrealEd->DeleteActors(TArray{Actor}, ActorWorld, GUnrealEd->GetSelectedActors()->GetElementSelectionSet());
					}
					else
#endif
					{
						ActorWorld->DestroyActor(Actor);
					}
				}
			}
			else
			{
#if WITH_EDITOR
				// Save the actor to the transaction buffer to support undo/redo, but do
				// not call Modify, as we do not want to dirty the actor's package and
				// we're only editing temporary, transient values
				SaveToTransactionBuffer(Actor, false);
				Actor->SetIsTemporarilyHiddenInEditor(true);
#endif
			}
		}
		if (bDelete)
		{
			ToolManager->EndUndoTransaction();
		}
		else
		{
#if WITH_EDITOR
			ToolManager->EndUndoTransaction();
#endif
		}
	}
}


void UPhysicBasedScatterTool::HandleCancel()
{
	// Tell The engine to start simulating physics
	if (ULevelEditorSubsystem* LevelEditorSubsystem = GEditor->GetEditorSubsystem<ULevelEditorSubsystem>())
	{
		LevelEditorSubsystem->EditorRequestEndPlay();
	}

	for (auto Item : SpawnedActors)
	{
		Item->Destroy();
	}
}

#pragma region PHYSICAL TOOL LAYOUT
FString UPhysicBasedScatterTool::GetCurrentLayoutMode() const
{
	return PropertySet ? ELayoutMode::Paint : ELayoutMode::Paint;
}

bool UPhysicBasedScatterTool::ShowModeWidgets() const
{
	if (GetCurrentLayoutMode() == ELayoutMode::Transform)
	{
		return true;
	}

	return false;
}

bool UPhysicBasedScatterTool::UsesTransformWidget() const
{
	if (GetCurrentLayoutMode() == ELayoutMode::Transform)
	{
		return true;
	}

	return false;
}

FVector UPhysicBasedScatterTool::GetWidgetLocation() const
{
	if (LastSelectedActors.Num() > 0 && LastSelectedActors.Last())
	{
		if (!LastSelectedActors.Last()->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed | RF_MirroredGarbage))
		{
			return LastSelectedActors.Last()->GetActorLocation();
		}
	}
	
	return FVector::ZeroVector;
}


void UPhysicBasedScatterTool::RegisterBrush()
{
	if (Brush && !Brush->IsRegistered())
	{
		Brush->RegisterComponentWithWorld(GetWorld());
		auto PickedMesh = PropertySet->GetRandomMesh();

		if (IsValid(PickedMesh))
		{
			Brush->SetStaticMesh(PickedMesh);
			Brush->SetVisibility(true);
		}
	}
}

void UPhysicBasedScatterTool::OnLayoutModeChange(FString InMode)
{
	if (InMode == ELayoutMode::PaintSelect || InMode == ELayoutMode::Select)
	{
		bSimulatePhysic = false;
		// auto Solver = GetWorld()->GetPhysicsScene()->GetSolver();
		// Solver->CompleteSceneSimulation();

	}
	else
	{
		bSimulatePhysic = true;
	}
	if (InMode == ELayoutMode::Transform)
	{
		GEditor->NoteSelectionChange();
	}
}

FVector UPhysicBasedScatterTool::GetPosition()
{
	return BrushPosition + (BrushNormal * PropertySet->NormalDistance) + PropertySet->GetPositionRandom();
}

TArray<UPrimitiveComponent*> UPhysicBasedScatterTool::GetPrimitives(const AActor* InActor)
{
	TArray<UPrimitiveComponent*> Comps;
	InActor->GetComponents<UPrimitiveComponent>(Comps, false);
	TArray<UPrimitiveComponent*> prims;

	for (auto Comp : Comps)
	{
		if (Comp->IsA<UPrimitiveComponent>())
		{
			UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp);
			if (Prim)
			{
				prims.Add(Prim);
			}
		}
	}

	return prims;
}

void UPhysicBasedScatterTool::DestroyActors(bool InSelected)
{
	TArray<AActor*> Actors;

	if (InSelected)
	{
		Actors = GetSelectedActors();
	}
	else
	{
		Actors = GetSpawnedActors();
	}

	for (auto& Actor : Actors)
	{
		SpawnedActors.Remove(Actor);
		LevelActors.Remove(Actor);
		Actor->Destroy();
	}
}

TArray<UPrimitiveComponent*> UPhysicBasedScatterTool::GetSelectedPrimitives()
{
	TArray<UPrimitiveComponent*> Prims;
	USelection* SelectedActors = GEditor->GetSelectedActors();

	// For each selected actor
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		if (AActor* LevelActor = Cast<AActor>(*Iter))
		{

			auto comps = LevelActor->GetComponents();
			TArray<UPrimitiveComponent*> prims;

			for (auto& comp : comps)
			{
				if (!comp->IsA<UGeometryCollectionComponent>())
				{
					if (comp->IsA<UPrimitiveComponent>())
					{
						UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(comp);
						if (Prim)
						{
							Prims.Add(Prim);
						}
					}
				}
			}
		}
	}

	return Prims;
}

void UPhysicBasedScatterTool::UnregisterBrush()
{
	if (Brush && Brush->IsRegistered())
	{
		Brush->SetVisibility(false);
		Brush->UnregisterComponent();

	}
}

TArray<AActor*> UPhysicBasedScatterTool::GetSelectedActors()
{
	TArray<AActor*> Actors;
	USelection* SelectedActors = GEditor->GetSelectedActors();

	// For each selected actor
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		if (AActor* LevelActor = Cast<AActor>(*Iter))
		{
			Actors.Add(LevelActor);
		}
	}

	return Actors;
}

void UPhysicBasedScatterTool::UpdatePhysics(AActor* InActor, bool bInEnableGravity)
{
	auto Prims = GetPrimitives(InActor);
	for (auto& Prim : Prims)
	{
		if (!Mobilities.Contains(Prim))
		{
			Mobilities.Add(Prim, Prim->Mobility.GetValue());
		}
		if (!Physics.Contains(Prim))
		{
			Physics.Add(Prim, Prim->IsSimulatingPhysics());
		}
		if (!Gravities.Contains(Prim))
		{
			Gravities.Add(Prim, Prim->IsGravityEnabled());
		}
		if (!Positions.Contains(Prim))
		{
			Positions.Add(Prim, Prim->GetComponentLocation());
		}
		if (!Rotations.Contains(Prim))
		{
			Rotations.Add(Prim, Prim->GetComponentRotation());
		}

		Prim->SetMobility(EComponentMobility::Movable);
		Prim->SetEnableGravity(bInEnableGravity);
		Prim->SetSimulatePhysics(bSimulatePhysic);

	}
}

void UPhysicBasedScatterTool::MakeSelectedStatic()
{
	auto Actors = GetSelectedActors();
	for (auto& Actor : Actors)
	{
		GEditor->SelectActor(Actor, false, true);

		auto Prims = GetPrimitives(Actor);
		for (auto& Prim : Prims)
		{
			ResetPrimitivePhysics(Prim, false, true);
		}
		
		SpawnedActors.Remove(Actor);
		LastSpawnedActors.Remove(Actor);
	}
}

void UPhysicBasedScatterTool::SelectPlacedActors(UStaticMesh* InStaticMesh)
{
	GEditor->SelectNone(false, true, false);
	if (InStaticMesh)
	{
		auto Components = GetSpawnedComponents();
		for (auto& Component : Components)
		{
			UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
			if (StaticMeshComponent)
			{
				if (StaticMeshComponent->GetStaticMesh() == InStaticMesh)
				{
					GEditor->SelectActor(Component->GetOwner(),true, false);
				}
			}
		}
	}
	else
	{
		for (auto& Actor : SpawnedActors)
		{
			GEditor->SelectActor(Actor,true, false);
		}
	}
}

void UPhysicBasedScatterTool::AddSelectedActor(AActor* InActor)
{
	OnLevelActorsAdded(InActor);
}

void UPhysicBasedScatterTool::CachePhysics()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), LevelActors);
	for (auto& actor : LevelActors)
	{
		if (!actor->IsA<AGeometryCollectionActor>())
		{

			auto Prims = GetPrimitives(actor);
			for (auto& Prim : Prims)
			{
				if (!Mobilities.Contains(Prim))
				{
					Mobilities.Add(Prim, Prim->Mobility.GetValue());
				}

				if (!Physics.Contains(Prim))
				{
					Physics.Add(Prim, Prim->IsSimulatingPhysics());
				}

				if (!Gravities.Contains(Prim))
				{
					Gravities.Add(Prim, Prim->IsGravityEnabled());
				}

				if (!Positions.Contains(Prim))
				{
					Positions.Add(Prim, Prim->GetComponentLocation());
				}

				if (!Rotations.Contains(Prim))
				{
					Rotations.Add(Prim, Prim->GetComponentRotation());
				}

				Prim->SetMobility(EComponentMobility::Static);
			}
		}
	}
}
void UPhysicBasedScatterTool::UpdateSelectionPhysics()
{
	if (!bSimulatePhysic)
	{
		return;
	}
	CachePhysics();

	auto Prims = GetSelectedPrimitives();
	for (auto& Prim : Prims)
	{
		if (!Mobilities.Contains(Prim))
		{
			Mobilities.Add(Prim, Prim->Mobility.GetValue());
		}

		if (!Physics.Contains(Prim))
		{
			Physics.Add(Prim, Prim->IsSimulatingPhysics());
		}

		if (!Gravities.Contains(Prim))
		{
			Gravities.Add(Prim, Prim->IsGravityEnabled());
		}

		Prim->SetSimulatePhysics(bSimulatePhysic);
		Prim->SetEnableGravity(false);
		Prim->SetMobility(EComponentMobility::Movable);
	}
}

void UPhysicBasedScatterTool::ResetTransform()
{
	USelection* SelectedActors = GEditor->GetSelectedActors();

	auto Prims = GetSelectedPrimitives();
	for (auto& Prim : Prims)
	{
		ResetPrimitivePhysics(Prim, true);
	}

	SelectedActors->DeselectAll();
}

TArray<UPrimitiveComponent*> UPhysicBasedScatterTool::GetSpawnedComponents()
{ 
	TArray<UPrimitiveComponent*> Components;
	for (auto& Actor : SpawnedActors)
	{
		Components.Append(GetPrimitives(Actor));
	}
	return Components;
}

void UPhysicBasedScatterTool::ResetPhysics()
{
	TArray<UPrimitiveComponent*> Prims;
	Mobilities.GetKeys(Prims);

	for (auto& Prim : Prims)
	{
		ResetPrimitivePhysics(Prim, false);
	}
}

void UPhysicBasedScatterTool::ResetPrimitivePhysics(UPrimitiveComponent* InPrim, bool bResetTransform, bool bForceStatic)
{
	if (InPrim && IsValid(InPrim) && !InPrim->IsBeingDestroyed()
		&& !InPrim->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed | RF_MirroredGarbage)
		&& InPrim->GetFName().IsValid())
	{
		if (Physics.Contains(InPrim))
		{
			InPrim->SetSimulatePhysics(bForceStatic ? false : Physics[InPrim]);
		}

		if (Gravities.Contains(InPrim))
		{
			InPrim->SetEnableGravity(bForceStatic ? false : Gravities[InPrim]);
		}

		if (Mobilities.Contains(InPrim))
		{
			InPrim->SetMobility(bForceStatic ? EComponentMobility::Type::Static : Mobilities[InPrim]);
		}

		if (bResetTransform)
		{
			if (Positions.Contains(InPrim))
			{
				InPrim->SetWorldLocation(Positions[InPrim]);
			}

			if (Rotations.Contains(InPrim))
			{
				InPrim->SetWorldRotation(Rotations[InPrim]);
			}
		}
	}

}

#pragma endregion


void UPhysicsDropPropertySet::SetRandomMesh()
{
	if (IsUseSelected())
	{
		// TODO : Use the selected actors from the viewport and compare them to items to drop array
		if (SelectedMeshIndex >= 0)
		{
			PickedMesh = ItemsToDrop[SelectedMeshIndex].Mesh;
		}
	}
	else
	{
		auto Meshes = ItemsToDrop;
		int Rand = FMath::FRandRange(0.0f, 100.0f);

		auto PickedMeshes = Meshes.FilterByPredicate([Rand](const FReferenceMeshData& InMesh)
			{
				return (InMesh.Mesh && InMesh.Chance >= Rand);
			}
		);
		if (PickedMeshes.Num() == 0)
		{
			return;
		}
		PickedMesh = PickedMeshes[FMath::RandRange(0, PickedMeshes.Num() - 1)].Mesh;
	}

	PositionRandom = FMath::RandPointInBox(FBox(MinPositionRandom, MaxPositionRandom));
	FVector RandRot = FMath::RandPointInBox(FBox(MinRotateRandom.Euler(), MaxRotateRandom.Euler()));
	RotateRandom = FRotator(RandRot.X, RandRot.Y, RandRot.Z);
	
	if (IsMinScaleLock() && IsMaxScaleLock())
	{
		ScaleRandom = FVector::OneVector * FMath::RandRange(MinScaleRandom.X, MaxScaleRandom.X);
	}
	else
	{
		ScaleRandom = FMath::RandPointInBox(FBox(MinScaleRandom, MaxScaleRandom));
	}
}

#undef LOCTEXT_NAMESPACE
