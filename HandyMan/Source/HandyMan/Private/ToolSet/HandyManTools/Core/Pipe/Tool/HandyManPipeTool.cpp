// Fill out your copyright notice in the Description page of Project Settings.


#include "HandyManPipeTool.h"

#include "ModelingToolTargetUtil.h"
#include "ToolBuilderUtil.h"
#include "ToolTargetManager.h"
#include "Components/SplineComponent.h"
#include "ToolSet/Core/HandyManSubsystem.h"
#include "ToolSet/HandyManTools/Core/Pipe/Actor/HandyManPipeActor.h"


#define LOCTEXT_NAMESPACE "PipeTool"


using namespace UE::Geometry;

#pragma region BUILDER
UHandyManPipeToolBuilder::UHandyManPipeToolBuilder(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	AcceptedClasses.Empty();
	AcceptedClasses.Add(USplineComponent::StaticClass());
	MinRequiredMatches = 1;
	MatchLimit = -1;
}

bool UHandyManPipeToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	// TODO : For now if you select a handy man actor this tool will ignore it and not build. In the future this should tell the tool to not spawn new actors and instead used the ones passed in.
	for (const auto& Item : SceneState.SelectedActors)
	{
		if (Item->IsA(AHandyManPipeActor::StaticClass()))
		{
			return false;
		}
	}
	
	int32 NumSplines = ToolBuilderUtil::CountComponents(SceneState, [&](UActorComponent* Object) -> bool
	{
		return Object->IsA<USplineComponent>();
	});
	FIndex2i SupportedRange = GetSupportedSplineCountRange();
	return (NumSplines >= SupportedRange.A && (SupportedRange.B == -1 || NumSplines <= SupportedRange.B));
}

void UHandyManPipeToolBuilder::SetupTool(const FToolBuilderState& SceneState, UInteractiveTool* Tool) const
{
	UHandyManPipeTool* NewTool = Cast<UHandyManPipeTool>(Tool);
	NewTool->SetTargetActors(SceneState.SelectedActors);
	OnSetupTool(NewTool, SceneState.SelectedActors, SceneState.SelectedComponents);
}
#pragma endregion


#pragma region TOOL

UHandyManPipeTool::UHandyManPipeTool()
{
	ToolName = LOCTEXT("PipeToolName", "Pipe");
	ToolTooltip = LOCTEXT("PipeToolDescription", "Sweeps circular geometry along an input spline");
	ToolLongName = LOCTEXT("PipeToolLongName", "Pipe Tool");
	ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
	ToolStartupRequirements = EScriptableToolStartupRequirements::Custom;
	CustomToolBuilderClass = UHandyManPipeToolBuilder::StaticClass();
}

void UHandyManPipeTool::SetTargetActors(const TArray<AActor*>& InActors)
{
	TargetActors = InActors;
}

void UHandyManPipeTool::SpawnOutputActorInstance(const UHandyManPipeToolProperties* InSettings)
{
	if (GetHandyManAPI() && InSettings)
	{
		for (const auto& Target : TargetActors)
		{
			TArray<USplineComponent*> Splines;
			Target->GetComponents<USplineComponent>(Splines);

			for (const USplineComponent* TargetSpline : Splines)
			{
				// Generate the splines from the input actor
				FActorSpawnParameters Params = FActorSpawnParameters();
				Params.ObjectFlags = RF_Transactional;
				FString name = FString::Format(TEXT("HandyMan_{0}"), { "Pipe" });
				FName UniqueName = MakeUniqueObjectName(nullptr, AHandyManPipeActor::StaticClass(), FName(*name));
				Params.Name = UniqueName;
				Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;

				auto World = GetToolWorld();
				auto ClassToSpawn = GetHandyManAPI()->GetPCGActorClass(FName(ToolName.ToString()));

				if (ClassToSpawn == nullptr)
				{
					ClassToSpawn = AHandyManPipeActor::StaticClass();
				}
				
				if (auto SpawnedActor =  World->SpawnActor<AHandyManPipeActor>(ClassToSpawn, Params))
				{
					
					// This is also editor-only: it's the label that shows up in the hierarchy
					FActorLabelUtilities::SetActorLabelUnique(SpawnedActor, UniqueName.ToString());
					// Initalize the actor
					SpawnedActor->SetActorTransform(TargetSpline->GetComponentTransform());
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

					SpawnedActor->SplineComponent->SetClosedLoop(TargetSpline->IsClosedLoop());
					SpawnedActor->SplineComponent->ClearSplinePoints();

					for (int i = 0; i < TargetSpline->GetNumberOfSplinePoints(); ++i)
					{
						SpawnedActor->SplineComponent->AddPoint(TargetSpline->GetSplinePointAt(i, ESplineCoordinateSpace::Local), false);
					}

					SpawnedActor->SplineComponent->UpdateSpline();
					
					

			
					OutputActorMap.Add(SpawnedActor->GetFName(), SpawnedActor);
					SelectionArray.Add(SpawnedActor);

					SpawnedActor->RerunConstructionScripts();
				}
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

	Settings->WatchProperty(Settings->PipeMaterial, [this](TObjectPtr<UMaterialInterface>)
	{
		for (const auto& Item : SelectionArray)
		{
			Item->PipeMaterial = Settings->PipeMaterial;
			Item->RerunConstructionScripts();
		}
	});
	
	Settings->WatchProperty(Settings->ShapeRadius, [this](float)
	{
		for (const auto& Item : SelectionArray)
		{
			Item->ShapeRadius = Settings->ShapeRadius;
			Item->RerunConstructionScripts();
		}
	});
	
	Settings->WatchProperty(Settings->RotationAngleDeg, [this](float)
	{
		for (const auto& Item : SelectionArray)
		{
			Item->RotationAngleDeg = Settings->RotationAngleDeg;
			Item->RerunConstructionScripts();
		}
	});
	
	Settings->WatchProperty(Settings->ShapeSegments, [this](int32)
	{
		for (const auto& Item : SelectionArray)
		{
			Item->ShapeSegments = Settings->ShapeSegments;
			Item->RerunConstructionScripts();
		}
	});
	Settings->WatchProperty(Settings->PolygroupMode, [this](EGeometryScriptPrimitivePolygroupMode)
	{
		for (const auto& Item : SelectionArray)
		{
			Item->PolygroupMode = Settings->PolygroupMode;
			Item->RerunConstructionScripts();
		}
	});
	
	Settings->WatchProperty(Settings->SampleSize, [this](int32)
	{
		for (const auto& Item : SelectionArray)
		{
			Item->SampleSize = Settings->SampleSize;
			Item->RerunConstructionScripts();
		}
	});
	
	Settings->WatchProperty(Settings->bProjectPointsToSurface, [this](bool)
	{
		for (const auto& Item : SelectionArray)
		{
			Item->bProjectPointsToSurface = Settings->bProjectPointsToSurface;
			Item->RerunConstructionScripts();
		}
	});
	
	Settings->WatchProperty(Settings->UVMode, [this](EGeometryScriptPrimitiveUVMode)
	{
		for (const auto& Item : SelectionArray)
		{
			Item->UVMode = Settings->UVMode;
			Item->RerunConstructionScripts();
		}
	});
	
	Settings->WatchProperty(Settings->StartEndRadius, [this](FVector2D)
	{
		for (const auto& Item : SelectionArray)
		{
			Item->StartEndRadius = Settings->StartEndRadius;
			Item->RerunConstructionScripts();
		}
	});
	
	Settings->SilentUpdateWatched();
	
	SpawnOutputActorInstance(Settings);

	
}

void UHandyManPipeTool::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);

	switch (ShutdownType)
	{
	case EToolShutdownType::Completed:
		break;
	case EToolShutdownType::Accept:
		{
			for (const auto& Item : TargetActors)
			{
				Item->Destroy();
			}
		}
		break;
	case EToolShutdownType::Cancel:
		for (const auto&  Item : OutputActorMap)
		{
			Item.Value->Destroy();
		}
		break;
	}
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
	
	
	switch (Button)
	{
	case EScriptableToolMouseButton::LeftButton:
		{
			FHitResult Hit;
			const bool bWasHit = Trace(Hit, DragPos);
			// TODO : This should be tracing for Pipe Actors and adding them to our current edit selection array.
		}
		break;
	case EScriptableToolMouseButton::RightButton:
		break;
	case EScriptableToolMouseButton::MiddleButton:
		{
			TArray<FHitResult> OutHits;
			const bool bWasHit = Trace(OutHits, DragPos);
		}
		
		break;
	}
}

void UHandyManPipeTool::OnDragBegin(const FInputDeviceRay& StartPosition, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button)
{
	
	FHitResult Hit;
	const bool bWasHit = Trace(Hit, StartPosition);
	
	if (Button == EScriptableToolMouseButton::LeftButton)
	{	
		// TODO: Cache the initial location so we can use it in a delta calculation
	}
	
}

void UHandyManPipeTool::OnDragEnd(const FInputDeviceRay& EndPosition, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button)
{

	if (Button == EScriptableToolMouseButton::LeftButton)
	{
		
		FHitResult Hit;
		const bool bWasHit = Trace(Hit, EndPosition);

		// TODO: Cache the final location to use in a delta calculation 
	}
}

FInputRayHit UHandyManPipeTool::CanClickFunc_Implementation(const FInputDeviceRay& PressPos, const EScriptableToolMouseButton& Button)
{

	FVector Start = PressPos.WorldRay.Origin;
	FVector End = PressPos.WorldRay.Origin + PressPos.WorldRay.Direction * HALF_WORLD_MAX;
	FHitResult Hit;

	if (Button == EScriptableToolMouseButton::LeftButton)
	{
		
		if (GetToolWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			if (!IsValid(Hit.GetActor()))
			{
				return UScriptableToolsUtilityLibrary::MakeInputRayHit_Miss();
			}

			if (IsShiftDown() && !IsCtrlDown())
			{
				if (Hit.GetActor()->IsA(AHandyManPipeActor::StaticClass()))
				{
					// Add this actor to our selection array
					SelectionArray.Add(Cast<AHandyManPipeActor>(Hit.GetActor()));
					return UScriptableToolsUtilityLibrary::MakeInputRayHit(Hit.Distance, nullptr);
				}
			}
			else if (IsCtrlDown() && !IsShiftDown())
			{
				if (Hit.GetActor()->IsA(AHandyManPipeActor::StaticClass()) && SelectionArray.Contains(Cast<AHandyManPipeActor>(Hit.GetActor())))
				{
					// Add this actor to our selection array
					SelectionArray.Remove(Cast<AHandyManPipeActor>(Hit.GetActor()));
					return UScriptableToolsUtilityLibrary::MakeInputRayHit(Hit.Distance, nullptr);
				}
			}

			
				
		}
	}
	else if (Button == EScriptableToolMouseButton::RightButton)
	{
		// Clear all
		if (IsCtrlDown() && !IsShiftDown())
		{
			SelectionArray.Empty();
		}
	}
	
	
	return UScriptableToolsUtilityLibrary::MakeInputRayHit_Miss();

	
}

void UHandyManPipeTool::OnHitByClickFunc(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& MouseButton)
{
	/*FVector Start = ClickPos.WorldRay.Origin;
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
	}*/
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

	if (Settings)
	{
		Settings->ShapeRadius += 0.25f;
	}
	
}

void UHandyManPipeTool::OnMouseWheelDown(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	if (Settings)
	{
		Settings->ShapeRadius -= 0.25f;
	}
}

void UHandyManPipeTool::OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform)
{
	Super::OnGizmoTransformChanged_Handler(GizmoIdentifier, NewTransform);
}

void UHandyManPipeTool::OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform, EScriptableToolGizmoStateChangeType ChangeType)
{
	Super::OnGizmoTransformStateChange_Handler(GizmoIdentifier, CurrentTransform, ChangeType);
	
}
#pragma endregion

#undef LOCTEXT_NAMESPACE