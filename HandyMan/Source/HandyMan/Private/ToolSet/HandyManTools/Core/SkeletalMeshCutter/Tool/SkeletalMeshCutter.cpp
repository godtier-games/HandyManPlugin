// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletalMeshCutter.h"

#include "HandyManSettings.h"
#include "Selection.h"
#include "UnrealEdGlobals.h"
#include "BaseGizmos/CombinedTransformGizmo.h"
#include "Components/ShapeComponent.h"
#include "Editor/UnrealEdEngine.h"
#include "ToolSet/HandyManTools/Core/SkeletalMeshCutter/Actor/SkeletalMeshCutterActor.h"



USkeletalMeshCutter::USkeletalMeshCutter()
{
	ToolName = NSLOCTEXT("USkeletalMeshCutter", "ToolName", "Skeletal Mesh Cutter");
	ToolCategory = NSLOCTEXT("USkeletalMeshCutter", "ToolCategory", "Mesh Edit");
	ToolTooltip = NSLOCTEXT("USkeletalMeshCutter", "ToolTooltip", "Cut up a skeletal mesh into FPS arms or legs using collision shapes");
	ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
	ToolLongName = NSLOCTEXT("USkeletalMeshCutter", "ToolName", "Skeletal Mesh Cutter");
}

void USkeletalMeshCutter::Setup()
{
	Super::Setup();
	
	EToolsFrameworkOutcomePins PropertyCreationOutcome;
	Settings = Cast<USkeletalMeshCutterPropertySet>(AddPropertySetOfType(USkeletalMeshCutterPropertySet::StaticClass(), "Settings", PropertyCreationOutcome));

	UHandyManSettings* HandyManSettings = GetMutableDefault<UHandyManSettings>();
	if (!Settings->MeshData.IsValid() && HandyManSettings && !HandyManSettings->GetToolsWithBlockedDialogs().Contains(FName(ToolName.ToString())))
	{
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
			NSLOCTEXT
			(
				"UBuildingGeneratorTool",
				"ErrorMessage",
				"This tool will automatically initialize the mesh to cut when you have an Input Mesh & you have set a name to save the new asset as. Disable this popup by adding this tool's name to the BlockedDialogsArray in the Handy Man Settings"
			));
	}


	LastCutterArray = Settings->Cutters;
	
	Settings->WatchProperty(Settings->Cutters, [this](TArray<ECutterShapeType>)
	{
		if (IsValid(OutputActor))
		{
			if (LastCutterArray.Num() != Settings->Cutters.Num())
			{
				if (LastCutterArray.Num() < Settings->Cutters.Num())
				{
					const FVector CutterLocation = OutputActor->AddNewCutter(LastCutterArray.Num(), Settings->Cutters[LastCutterArray.Num()]);
					// Create a gizmo at this cutters location
					EToolsFrameworkOutcomePins Outcome;
					CreateTRSGizmo(FString::FromInt(LastCutterArray.Num()), FTransform(CutterLocation), FScriptableToolGizmoOptions(),Outcome);
					SetGizmoVisible(FString::FromInt(LastCutterArray.Num()), false);
					LastCutterArray = Settings->Cutters;
				}
				else
				{
					// Create a gizmo at this cutters location
					EToolsFrameworkOutcomePins Outcome;
					DestroyTRSGizmo(FString::FromInt(LastCutterArray.Num()), Outcome);
					OutputActor->RemoveCutter(LastCutterArray.Num() - 1);
					LastCutterArray = Settings->Cutters;
				}
			}
		}
	});

	
	SpawnOutputActorInstance(Settings, FTransform::Identity);

	// First hide unselected as this will also hide group actor members
	GUnrealEd->edactHideUnselected( GetToolWorld() );
	// Then unhide selected to ensure that everything that's selected will be unhidden
	GUnrealEd->edactUnhideSelected(GetToolWorld());


	GEditor->GetSelectedActors()->DeselectAll();
	
}

void USkeletalMeshCutter::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);

	GEditor->GetSelectedActors()->DeselectAll();
	GUnrealEd->edactUnHideAll(GetToolWorld());

	switch (ShutdownType)
	{
	case EToolShutdownType::Completed:
		break;
	case EToolShutdownType::Accept:
		if (OutputActor)
		{
			OutputActor->SaveObject();
		}
		break;
	case EToolShutdownType::Cancel:
		if (OutputActor)
		{
			OutputActor->RemoveCreatedAsset();
		}
		break;
	}

	/*for (const auto& Viewport : GUnrealEd->GetAllViewportClients())
	{
		if(!Viewport->IsFocused(GEditor->GetActiveViewport())) continue;
		Viewport->SetViewMode(VMI_Lit);
	}*/
}

bool USkeletalMeshCutter::CanAccept() const
{
	return Settings && Settings->MeshData.IsValid();
}

void USkeletalMeshCutter::SpawnOutputActorInstance(const USkeletalMeshCutterPropertySet* InSettings, const FTransform& SpawnTransform)
{
	GEditor->GetSelectedActors()->DeselectAll();
	
	if (GetHandyManAPI() && InSettings)
	{

		// Generate the splines from the input actor
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags = RF_Transactional;
		SpawnInfo.Name = FName("MeshCutter");

		auto World = GetToolWorld();
		auto ClassToSpawn = GetHandyManAPI()->GetPCGActorClass(FName(ToolName.ToString()));
		if (auto SpawnedActor =  World->SpawnActor<ASkeletalMeshCutterActor>(ClassToSpawn))
		{
			// Initalize the actor
			SpawnedActor->SetActorTransform(SpawnTransform);
			OutputActor = (SpawnedActor);
		}

		for (int i = 0; i < InSettings->Cutters.Num(); ++i)
		{
			const FVector& Location = OutputActor->AddNewCutter(i, InSettings->Cutters[i]);
			EToolsFrameworkOutcomePins Outcome;
			CreateTRSGizmo(FString::FromInt(i), FTransform(Location), FScriptableToolGizmoOptions(),Outcome);
		}

		
		
		GEditor->SelectActor(OutputActor, true, true);
		
		GEditor->MoveViewportCamerasToActor(*OutputActor, true);
		
		/*for (const auto& Viewport : GUnrealEd->GetAllViewportClients())
		{
			if(!Viewport->IsFocused(GEditor->GetActiveViewport())) continue;
			Viewport->SetViewMode(VMI_Unlit);
		}*/
	}
	else
	{
		// TODO : Error Dialogue
	}
}

FInputRayHit USkeletalMeshCutter::TestIfHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	FVector Start = ClickPos.WorldRay.Origin;
	FVector End = ClickPos.WorldRay.Origin + ClickPos.WorldRay.Direction * HALF_WORLD_MAX;
	FHitResult Hit;

	if (GetToolWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		if (Hit.Component.IsValid() && Hit.Component->IsA(UShapeComponent::StaticClass()))
		{
			return UScriptableToolsUtilityLibrary::MakeInputRayHit_MaxDepth();
		}
	}

	GEditor->GetSelectedActors()->DeselectAll();
	
	return UScriptableToolsUtilityLibrary::MakeInputRayHit_Miss();
}

void USkeletalMeshCutter::OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	FVector Start = ClickPos.WorldRay.Origin;
	FVector End = ClickPos.WorldRay.Origin + ClickPos.WorldRay.Direction * HALF_WORLD_MAX;
	FHitResult Hit;

	if (GetToolWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		if (Hit.GetActor())
		{
			if (Hit.Component.IsValid() && Hit.Component->IsA(UShapeComponent::StaticClass()) && Hit.Component->ComponentTags.Num() > 0)
			{
				// Activate this components gizmo
				HideAllGizmos();
				SetGizmoVisible(Hit.Component->ComponentTags[0].ToString(), true);
			}
		}
	}

	GEditor->GetSelectedActors()->DeselectAll();
}

void USkeletalMeshCutter::OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform)
{
	Super::OnGizmoTransformChanged_Handler(GizmoIdentifier, NewTransform);

	TArray<UShapeComponent*> Components;
	OutputActor->GetComponents(UShapeComponent::StaticClass(), Components);

	for (const auto& Component : Components)
	{
		if(!Component->ComponentHasTag(FName(*GizmoIdentifier))) continue;
		Component->SetWorldTransform(NewTransform);
		break;
	}

	OutputActor->RerunConstructionScripts();
}

void USkeletalMeshCutter::OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform,
	EScriptableToolGizmoStateChangeType ChangeType)
{
	Super::OnGizmoTransformStateChange_Handler(GizmoIdentifier, CurrentTransform, ChangeType);

	if(!OutputActor) return;
	
	if (ChangeType == EScriptableToolGizmoStateChangeType::EndTransform || ChangeType == EScriptableToolGizmoStateChangeType::UndoRedo)
	{
		TArray<UShapeComponent*> Components;
		OutputActor->GetComponents(UShapeComponent::StaticClass(), Components);

		for (const auto& Component : Components)
		{
			if(!Component->ComponentHasTag(FName(*GizmoIdentifier))) continue;
			Component->SetWorldTransform(CurrentTransform);
			break;
		}

		OutputActor->RerunConstructionScripts();
	}
}


void USkeletalMeshCutter::SaveDuplicate()
{

	OutputActor->SaveObject();
}

void USkeletalMeshCutter::HideAllGizmos()
{
	for (const auto& Gizmo : Gizmos)
	{
		Gizmo.Value->SetVisibility(false);
	}
}

///-------------------------------------------------------------
/// PROPERTY SET
///
#if WITH_EDITOR

void USkeletalMeshCutterPropertySet::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (!ParentTool.Get() || !ParentTool->IsA(USkeletalMeshCutter::StaticClass())) return;

	if (PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MeshData))
	{
		if (MeshData.IsValid())
		{
			if (Cast<USkeletalMeshCutter>(ParentTool.Get())->OutputActor)
			{
				Cast<USkeletalMeshCutter>(ParentTool.Get())->OutputActor->Initialize(MeshData);
			}
		}

		if (MeshData.SectionName.IsEmpty())
		{
			MeshData.SectionName = "PART";
		}

		if (MeshData.FolderName.IsEmpty())
		{
			MeshData.SectionName = "GENERATED";
		}
	}
}

#endif

USkeletalMeshCutterPropertySet::USkeletalMeshCutterPropertySet()
{
	Cutters.Add(ECutterShapeType::Box);
	Cutters.Add(ECutterShapeType::Box);
	Cutters.Add(ECutterShapeType::Sphere);
}


