// Fill out your copyright notice in the Description page of Project Settings.


#include "HandyManPipeTool.h"

#include "ToolBuilderUtil.h"
#include "Components/SplineComponent.h"
#include "ToolSet/Core/HandyManSubsystem.h"
#include "ToolSet/HandyManTools/Core/Pipe/Actor/HandyManPipeActor.h"


#define LOCTEXT_NAMESPACE "PipeTool"


using namespace UE::Geometry;

#pragma region BUILDER
UHandyManPipeToolBuilder::UHandyManPipeToolBuilder()
{
	AcceptedClasses.Empty();
	AcceptedClasses.Add(USplineComponent::StaticClass());
	MinRequiredMatches = 1;
	MatchLimit = -1;
}

bool UHandyManPipeToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	int32 NumSplines = ToolBuilderUtil::CountComponents(SceneState, [&](UActorComponent* Object) -> bool
		{
			return Object->IsA<USplineComponent>();
		});
	FIndex2i SupportedRange = GetSupportedSplineCountRange();
	return (NumSplines >= SupportedRange.A && (SupportedRange.B == -1 || NumSplines <= SupportedRange.B));
}

void UHandyManPipeToolBuilder::SetupTool(const FToolBuilderState& SceneState, UInteractiveTool* Tool) const
{
	Super::SetupTool(SceneState, Tool);
}
#pragma endregion


#pragma region TOOL

UHandyManPipeTool::UHandyManPipeTool()
{
	ToolName = LOCTEXT("PipeToolName", "Pipe");
	ToolTooltip = LOCTEXT("PipeToolDescription", "Sweeps circular geometry along an input spline");
	ToolLongName = LOCTEXT("PipeToolLongName", "Generate Building From Blockout");
	ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
	ToolStartupRequirements = EScriptableToolStartupRequirements::Custom;
	CustomToolBuilderClass = UHandyManPipeToolBuilder::StaticClass();
}

void UHandyManPipeTool::SpawnOutputActorInstance(const UHandyManPipeToolProperties* InSettings, const FTransform& SpawnTransform)
{
	if (GetHandyManAPI() && InSettings)
	{
		for (const auto& Target : TargetActors)
		{
			const USplineComponent* TargetSpline = Target->FindComponentByClass<USplineComponent>();
			// Generate the splines from the input actor
			FActorSpawnParameters Params = FActorSpawnParameters();
			Params.ObjectFlags = RF_Transactional;
			FString name = FString::Format(TEXT("Actor_{0}"), { "Pipe" });
			FName fname = MakeUniqueObjectName(nullptr, AHandyManPipeActor::StaticClass(), FName(*name));
			Params.Name = fname;
			Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;

			auto World = GetToolWorld();
			auto ClassToSpawn = GetHandyManAPI()->GetPCGActorClass(FName(ToolName.ToString()));
			if (auto SpawnedActor =  World->SpawnActor<AHandyManPipeActor>(ClassToSpawn, Params))
			{
				// Initalize the actor
				SpawnedActor->SetActorTransform(Target->GetActorTransform());
				SpawnedActor->GetDynamicMeshComponent()->SetMaterial(0, InSettings->PipeMaterial);
				SpawnedActor->PipeMaterial = InSettings->PipeMaterial;
				SpawnedActor->bFlipOrientation = InSettings->bFlipOrientation;
				SpawnedActor->PolygroupMode = InSettings->PolygroupMode;
				SpawnedActor->SampleSize = InSettings->SampleSize;
				SpawnedActor->ShapeRadius = InSettings->ShapeRadius;
				SpawnedActor->ShapeSegments = InSettings->ShapeSegments;
				SpawnedActor->RotationAngleDeg = InSettings->RotationAngleDeg;
				SpawnedActor->StartEndRadius = InSettings->StartEndRadius;
				SpawnedActor->bProjectPointsToSurface = InSettings->bProjectPointsToSurface;
				SpawnedActor->UVMode = InSettings->UVMode;

				for (int i = 0; i < TargetSpline->GetNumberOfSplinePoints(); ++i)
				{
					SpawnedActor->SplineComponent->AddPoint(TargetSpline->GetSplinePointAt(i, ESplineCoordinateSpace::World));

				}
			
				OutputActorMap.Add(SpawnedActor->GetFName(), SpawnedActor);

				SpawnedActor->RerunConstructionScripts();
			}
		}
		
		
	}
	else
	{
		// TODO : Error Dialogue
	}
}

void UHandyManPipeTool::Setup()
{
	Super::Setup();


	EToolsFrameworkOutcomePins Outcome;
	Settings = Cast<UHandyManPipeToolProperties>(AddPropertySetOfType(UHandyManPipeToolProperties::StaticClass(), TEXT("PipeSettings"), Outcome));
	
}

void UHandyManPipeTool::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);
}

FInputRayHit UHandyManPipeTool::TestCanHoverFunc_Implementation(const FInputDeviceRay& PressPos, const FScriptableToolModifierStates& Modifiers)
{
	return UScriptableToolsUtilityLibrary::MakeInputRayHit_MaxDepth();
}

void UHandyManPipeTool::OnBeginHover(const FInputDeviceRay& DevicePos, const FScriptableToolModifierStates& Modifiers)
{
}

bool UHandyManPipeTool::OnUpdateHover(const FInputDeviceRay& DevicePos, const FScriptableToolModifierStates& Modifiers)
{
	return true;
}

void UHandyManPipeTool::OnEndHover()
{
}

FInputRayHit UHandyManPipeTool::CanClickDrag_Implementation(const FInputDeviceRay& PressPos, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button)
{
	return UScriptableToolsUtilityLibrary::MakeInputRayHit_MaxDepth();
}

void UHandyManPipeTool::OnClickDrag(const FInputDeviceRay& DragPos, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button)
{
	LatestPosition = BrushPosition;
	
	switch (Button)
	{
	case EScriptableToolMouseButton::LeftButton:
		{
			FHitResult Hit;
			const bool bWasHit = Trace(Hit, DragPos);
			UpdateBrush(DragPos);
			Brush->SetVisibility(true);
			HideAllGizmos();
		}
		break;
	case EScriptableToolMouseButton::RightButton:
		break;
	case EScriptableToolMouseButton::MiddleButton:
		{
			TArray<FHitResult> OutHits;
			const bool bWasHit = Trace(OutHits, DragPos);

			for (const auto& Hit : OutHits)
			{
				if (Hit.GetActor() && Hit.GetActor()->IsA(AStaticMeshActor::StaticClass())
				&& Cast<AStaticMeshActor>(Hit.GetActor())->GetStaticMeshComponent()->GetStaticMesh()
				&& LastSpawnedActors.Contains(Hit.GetActor()))
				{
				
					const FName ActorName = Hit.GetActor()->GetFName();
					EToolsFrameworkOutcomePins OutCome;
					DestroyTRSGizmo(ActorName.ToString(), OutCome);

					if (!bIsEditing)
					{
						LastSpawnedActors.RemoveSingle(Hit.GetActor());
						Hit.GetActor()->Destroy();
					
						for (int i = 0; i < CachedOpenings.Openings.Num(); ++i)
						{
							if(!CachedOpenings.Openings[i].Mesh.GetFName().IsEqual(ActorName)) continue;
							CachedOpenings.Openings.RemoveAt(i);
							break;
						}

						if (OutputActor)
						{
							OutputActor->UpdatedGeneratedOpenings(CachedOpenings.Openings);
						}
					}
					else
					{
						SpawnedActorsToDestroy.Add(Hit.GetActor());
						Hit.GetActor()->SetIsTemporarilyHiddenInEditor(true);
						for (int i = 0; i < EditedOpenings.Openings.Num(); ++i)
						{
							if(!EditedOpenings.Openings[i].Mesh.GetFName().IsEqual(ActorName)) continue;
							EditedOpenings.Openings.RemoveAt(i);
							break;
						}

						if (OutputActor)
						{
							OutputActor->UpdatedGeneratedOpenings(EditedOpenings.Openings);
						}
					}
				
				}
			}
		}
		
		break;
	}
}

void UHandyManPipeTool::OnDragBegin(const FInputDeviceRay& StartPosition, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button)
{

	bIsPainting = true;
	
	auto PickedMesh = PaintingMesh;
	FHitResult Hit;
	const bool bWasHit = Trace(Hit, StartPosition);
	
	if (Button == EScriptableToolMouseButton::LeftButton)
	{
		if (IsValid(PickedMesh))
		{
			if (!IsValid(Brush))
			{
				return;
			}


			// If we are holding a modifier key cache the last hovered opening
			if (!bWasHit && (IsShiftDown() || IsCtrlDown()))
			{
				if (!bIsEditing)
				{
					if (Hit.GetActor() && CachedOpenings.Contains(Hit.GetActor()))
					{
						LastHoveredOpening = Hit.GetActor();
					}
				}
				else
				{
					if (Hit.GetActor() && EditedOpenings.Contains(Hit.GetActor()))
					{
						LastHoveredOpening = Hit.GetActor();
					}
				}

			}
			else if (bWasHit && !(IsShiftDown() || IsCtrlDown()))
			{
				LastHoveredOpening = nullptr;
			}
			

			const auto Extent = PickedMesh->GetBounds().BoxExtent;
				
			Brush->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));

			if (LastHoveredOpening)
			{
				if (IsShiftDown() || IsCtrlDown())
				{
					if (!bIsSnapping)
					{
						bIsSnapping = true;
						SnapStartLocation = LastHoveredOpening->GetActorLocation();
						SnapStartRotation = LastHoveredOpening->GetActorRotation();
						Brush->SetWorldLocation(SnapStartLocation);
						Brush->AddLocalOffset(FVector(Extent.X, 0, 0));
					}
				}
				else
				{
					bIsSnapping = false;
					SnapStartLocation = FVector::ZeroVector;
					SnapStartRotation = FRotator::ZeroRotator;
					Brush->SetWorldLocation(Hit.Location);
				}

				if (IsShiftDown() && !IsCtrlDown())
				{
					SnappingDirection = FVector(0, 1, 0);
					bIsSnappingOnRightAxis = true;
				}

				if (!IsShiftDown() && IsCtrlDown())
				{
					SnappingDirection = FVector(0, 0, 1);
					bIsSnappingOnRightAxis = false;
				}
			}
			else
			{
				Brush->SetWorldLocation(Hit.Location);
			}
			
			LastFrameScreenPosition = StartPosition.ScreenPosition;
		}
	
	}
	
}

void UHandyManPipeTool::OnDragEnd(const FInputDeviceRay& EndPosition, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button)
{
	if (!bIsPainting)
	{
		return;
	}

	if (Button == EScriptableToolMouseButton::LeftButton)
	{
		if (!PaintingMesh)
		{
			FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
				LOCTEXT("UHandyManPipeTool", "You are trying to add meshes to the building but no mesh is selected. Please select a mesh by dragging it into the scene from the asset or content browser."));
			return;
		}
			
		FHitResult Hit;
		const bool bWasHit = Trace(Hit, EndPosition);
		
		if (bWasHit && IsValid(PaintingMesh))
		{

			FDynamicOpening OpeningRef;

			for (int i = 0; i < Settings->Openings->Openings.Num(); ++i)
			{
				UObject* NextMesh = Settings->Openings->Openings[i].Mesh;

				if (!IsValid(NextMesh))
				{
					return;
				}

				if (NextMesh->IsA(AActor::StaticClass()))
				{
					TArray<UActorComponent*> ComponentsArray = Cast<AActor>(NextMesh)->K2_GetComponentsByClass(UStaticMeshComponent::StaticClass());

					if (IsValid(ComponentsArray[0]) && IsValid(Cast<UStaticMeshComponent>(ComponentsArray[0])->GetStaticMesh()))
					{
						if (Cast<UStaticMeshComponent>(ComponentsArray[0])->GetStaticMesh() == PaintingMesh)
						{
							OpeningRef = Settings->Openings->Openings[i];
							break;
						}
					}
				}
				else if (NextMesh->IsA(UStaticMesh::StaticClass()))
				{
					if (Cast<UStaticMesh>(NextMesh) == PaintingMesh)
					{
						OpeningRef = Settings->Openings->Openings[i];
						break;
					}
				}
			}
		
			bIsPainting = true;

			LastSpawnedPosition = Hit.Location;
			auto Rotation = Hit.Normal.Rotation();

			
			SpawnOpeningFromReference(OpeningRef, Rotation);
		}

		ResetBrush();
	}
}

FInputRayHit UHandyManPipeTool::CanClickFunc_Implementation(const FInputDeviceRay& PressPos, const EScriptableToolMouseButton& Button)
{
	if (!IsShiftDown() && !IsCtrlDown())
	{
		return UScriptableToolsUtilityLibrary::MakeInputRayHit_Miss();
	}

	
	
	FVector Start = PressPos.WorldRay.Origin;
	FVector End = PressPos.WorldRay.Origin + PressPos.WorldRay.Direction * HALF_WORLD_MAX;
	FHitResult Hit;

	if (GetToolWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		if (IsValid(Hit.GetActor()))
		{
			if (Hit.GetActor()->IsA(APCG_Pipe::StaticClass()))
			{
				ResetBrush();
				
				return UScriptableToolsUtilityLibrary::MakeInputRayHit(Hit.Distance, nullptr);
			}

			
			bool bHasHitAnOpening = false;
			if (!bIsEditing)
			{
				for (const auto& Opening : CachedOpenings.Openings)
				{
					if(!Opening.Mesh.GetFName().IsEqual(Hit.GetActor()->GetFName())) continue;
					bHasHitAnOpening = true;
				}
			}
			else
			{
				for (const auto& Opening : EditedOpenings.Openings)
				{
					if(!Opening.Mesh.GetFName().IsEqual(Hit.GetActor()->GetFName())) continue;
					bHasHitAnOpening = true;
				}
			}

			if (bHasHitAnOpening)
			{
				bIsCopying = false;
				return UScriptableToolsUtilityLibrary::MakeInputRayHit(Hit.Distance, nullptr);
			}
		}
	}
	
	return UScriptableToolsUtilityLibrary::MakeInputRayHit_Miss();

	
}

void UHandyManPipeTool::OnHitByClickFunc(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& MouseButton)
{
	FVector Start = ClickPos.WorldRay.Origin;
	FVector End = ClickPos.WorldRay.Origin + ClickPos.WorldRay.Direction * HALF_WORLD_MAX;
	FHitResult Hit;

	if (GetToolWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		if (IsValid(Hit.GetActor()))
		{
			FString Identifier = "";
			if (!bIsEditing)
			{
				for (const auto& Opening : CachedOpenings.Openings)
				{
					if(!Opening.Mesh.GetFName().IsEqual(Hit.GetActor()->GetFName())) continue;
					Identifier = Hit.GetActor()->GetFName().ToString();
				}
			}
			else
			{
				for (const auto& Opening : EditedOpenings.Openings)
				{
					if(!Opening.Mesh.GetFName().IsEqual(Hit.GetActor()->GetFName())) continue;
					Identifier = Hit.GetActor()->GetFName().ToString();
				}
			}

			if (Identifier.IsEmpty()) {return;}

			GetToolManager()->BeginUndoTransaction(LOCTEXT("EditOpening", "Edit Opening"));
			for (const auto& Gizmo : Gizmos)
			{
				if(!Identifier.Equals(Gizmo.Key))
				{
					Gizmo.Value->SetVisibility(false);
				}
				else
				{
					if (IsCtrlDown())
					{
						if (CopiedScale.Equals(FVector::Zero()) || IsShiftDown())
						{
							CopiedScale = Hit.GetActor()->GetActorRelativeScale3D();
							Brush->SetWorldScale3D(CopiedScale);
							GetToolManager()->DisplayMessage(LOCTEXT("CopiedScaleName", "Copied Scale"), EToolMessageLevel::UserMessage);
							return;
						}

						Hit.GetActor()->SetActorRelativeScale3D(CopiedScale);
						OutputActor->RerunConstructionScripts();
						Brush->SetWorldScale3D(FVector::One());
						GetToolManager()->DisplayMessage(LOCTEXT("PastedScaleName", "Pasted Scale"), EToolMessageLevel::UserMessage);
						return;
					}
					
					Gizmo.Value->SetVisibility(true);
					// Turn off the brush so the user can see the gizmo
					Brush->SetVisibility(false);
				}
			}

			GetToolManager()->EndUndoTransaction();
		}
	}
}

bool UHandyManPipeTool::MouseBehaviorModiferCheckFunc(const FInputDeviceState& InputDeviceState)
{
	return true;
}

FInputRayHit UHandyManPipeTool::CanUseMouseWheel_Implementation(const FInputDeviceRay& PressPos)
{
	// Only return a hit if the player is holding the shift modifier key else let them use the scroll how they would naturally

	if (IsShiftDown())
	{
		return UScriptableToolsUtilityLibrary::MakeInputRayHit_MaxDepth();
	}

	return FInputRayHit();
}

void UHandyManPipeTool::OnMouseWheelUp(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	// Get the current index of the selected mesh and increment it by 1 to select the next mesh
	// If this is the last mesh in the array then loop back to the first mesh

	if (Settings->Openings->Openings.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
			LOCTEXT("UHandyManPipeTool", "You do not have any openings set up in the settings. Please add some openings to the settings."));
		return;
	}

	int32 NextIndex = LastOpeningIndex + 1;
	
	if (NextIndex == Settings->Openings->Openings.Num())
	{
		NextIndex = 0;
	}

	LastOpeningIndex = NextIndex;

	// Get the mesh and spawn it as the type of actor necessary
	UObject* NextMesh = Settings->Openings->Openings[NextIndex].Mesh;

	if (!IsValid(NextMesh))
	{
		return;
	}

	if (NextMesh->IsA(AActor::StaticClass()))
	{
		// Get The static mesh component and set the brushes mesh to the static mesh
		TArray<UActorComponent*> ComponentsArray = Cast<AActor>(NextMesh)->K2_GetComponentsByClass(UStaticMeshComponent::StaticClass());

		if (IsValid(ComponentsArray[0]) && IsValid(Cast<UStaticMeshComponent>(ComponentsArray[0])->GetStaticMesh()))
		{
			PaintingMesh = Cast<UStaticMeshComponent>(ComponentsArray[0])->GetStaticMesh();
			Brush->SetStaticMesh(PaintingMesh);
			Brush->SetWorldScale3D(FVector::One());
		}
	}
	else if (NextMesh->IsA(UStaticMesh::StaticClass()))
	{
		// Get The static mesh and set the brushes mesh to the static mesh
		PaintingMesh = Cast<UStaticMesh>(NextMesh);
		Brush->SetStaticMesh(PaintingMesh);
		Brush->SetWorldScale3D(FVector::One());
	}
	else
	{
		// Error Dialogue
	}
	
}

void UHandyManPipeTool::OnMouseWheelDown(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	if (Settings->Openings->Openings.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
			LOCTEXT("UHandyManPipeTool", "You do not have any openings set up in the settings. Please add some openings to the settings."));
		return;
	}

	int32 NextIndex = LastOpeningIndex - 1;
	
	if (NextIndex < 0)
	{
		NextIndex = Settings->Openings->Openings.Num() - 1;
	}

	LastOpeningIndex = NextIndex;

	// Get the mesh and spawn it as the type of actor necessary
	UObject* NextMesh = Settings->Openings->Openings[NextIndex].Mesh;

	if (!IsValid(NextMesh))
	{
		return;
	}

	if (NextMesh->IsA(AActor::StaticClass()))
	{
		// Get The static mesh component and set the brushes mesh to the static mesh
		TArray<UActorComponent*> ComponentsArray = Cast<AActor>(NextMesh)->K2_GetComponentsByClass(UStaticMeshComponent::StaticClass());

		if (IsValid(ComponentsArray[0]) && IsValid(Cast<UStaticMeshComponent>(ComponentsArray[0])->GetStaticMesh()))
		{
			PaintingMesh = Cast<UStaticMeshComponent>(ComponentsArray[0])->GetStaticMesh();
			Brush->SetStaticMesh(PaintingMesh);
			Brush->SetWorldScale3D(FVector::One());
		}
	}
	else if (NextMesh->IsA(UStaticMesh::StaticClass()))
	{
		// Get The static mesh and set the brushes mesh to the static mesh
		PaintingMesh = Cast<UStaticMesh>(NextMesh);
		Brush->SetStaticMesh(PaintingMesh);
		Brush->SetWorldScale3D(FVector::One());
	}
	else
	{
		// Error Dialogue
	}
}

void UHandyManPipeTool::OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform)
{
	Super::OnGizmoTransformChanged_Handler(GizmoIdentifier, NewTransform);

	// TODO : This doesn't work currently it crashes the engine.
	if (Settings)
	{
		/*const FVector& Loc = NewTransform.GetLocation();
		const FVector& Offset = FVector::UpVector * 100.0f;
	
		FHitResult Hit;
	
		if (GetToolWorld()->LineTraceSingleByChannel(Hit, Loc + Offset, Loc - Offset, ECC_Visibility))
		{
			if (Hit.GetActor() && Hit.GetActor()->IsA(APCG_Pipe::StaticClass()))
			{
				SetGizmoTransform(GizmoIdentifier, FTransform(NewTransform.GetRotation(), Hit.Location, NewTransform.GetScale3D()), false);
			}
		}
		*/

		UpdateOpeningTransforms(GizmoIdentifier, NewTransform);
		OutputActor->RerunConstructionScripts();
		
	}
	
}

void UHandyManPipeTool::OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform, EScriptableToolGizmoStateChangeType ChangeType)
{
	Super::OnGizmoTransformStateChange_Handler(GizmoIdentifier, CurrentTransform, ChangeType);

	if (ChangeType == EScriptableToolGizmoStateChangeType::BeginTransform)
	{
		if (IsCtrlDown() && !bIsCopying)
		{
			if (!bIsEditing)
			{
				// Spawn another actor in the current location of the gizmo
				for (const auto& Opening : CachedOpenings.Openings)
				{
					if(!Opening.Mesh.GetFName().IsEqual(FName(*GizmoIdentifier))) continue;
					SpawnOpeningFromReference(Opening, false);
					//HideAllGizmos();
					bIsCopying = true;
					return;
				}
			}
			else
			{
				// Spawn another actor in the current location of the gizmo
				for (const auto& Opening : EditedOpenings.Openings)
				{
					if(!Opening.Mesh.GetFName().IsEqual(FName(*GizmoIdentifier))) continue;
					SpawnOpeningFromReference(Opening, false);
					//HideAllGizmos();
					bIsCopying = true;
					return;
				}
			}
		}
	}

	if (ChangeType == EScriptableToolGizmoStateChangeType::EndTransform || ChangeType == EScriptableToolGizmoStateChangeType::UndoRedo)
	{
	
		UpdateOpeningTransforms(GizmoIdentifier, CurrentTransform);
		bIsCopying = false;
		OutputActor->RerunConstructionScripts();
	}
	


}
#pragma endregion

#undef LOCTEXT_NAMESPACE