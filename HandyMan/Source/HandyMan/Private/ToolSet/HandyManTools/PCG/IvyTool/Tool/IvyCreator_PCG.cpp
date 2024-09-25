// Fill out your copyright notice in the Description page of Project Settings.


#include "IvyCreator_PCG.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Selection.h"
#include "ActorPartition/PartitionActor.h"
#include "Helpers/PCGGraphParametersHelpers.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "ToolSet/Core/HM_IvyToolMeshData.h"
#include "ToolSet/HandyManTools/PCG/IvyTool/Actor/PCG_IvyActor.h"

#define LOCTEXT_NAMESPACE "UIvyCreator_PCG"

UIvyCreator_PCG::UIvyCreator_PCG(): PropertySet(nullptr)
{
	ToolName = LOCTEXT("ToolName", "Vine Tool");
	ToolTooltip = LOCTEXT("Tooltip", "Procedurally Place Ivy on selected Static Meshes");
	ToolCategory = LOCTEXT("ToolCategory", "Flora");
	ToolLongName = LOCTEXT("LongToolName", "Vine Tool");
}

void UIvyCreator_PCG::Setup()
{
	Super::Setup();
	
	GEditor->GetSelectedActors()->DeselectAll();

	EToolsFrameworkOutcomePins PropertyCreationOutcome;
	PropertySet = Cast<UIvyCreator_PCG_PropertySet>(AddPropertySetOfType(UIvyCreator_PCG_PropertySet::StaticClass(), "Settings", PropertyCreationOutcome));

	PropertySet->WatchProperty(PropertySet->MeshData, [this](UHM_IvyToolMeshData*)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				IvyActor->SetVineMaterial(PropertySet->MeshData->VineMaterial);
				UPCGGraphParametersHelpers::SetObjectParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMeshData"), PropertySet->MeshData);
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->bDebugMeshPoints, [this](bool)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetBoolParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalDebug"), PropertySet->bDebugMeshPoints);
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->bVoxelizeMesh, [this](bool)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetBoolParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalVoxelizeMesh"), PropertySet->bVoxelizeMesh);
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->VoxelSize, [this](float)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalVoxelSize"), PropertySet->VoxelSize);
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->RandomSeed, [this](int32)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetInt32Parameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalSeed"), PropertySet->RandomSeed);
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->SplineCount, [this](int32)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetInt32Parameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalSplineCount"), PropertySet->SplineCount);
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->StartingPointsHeightRatio, [this](float)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalStartDistance"), PropertySet->StartingPointsHeightRatio);
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->PathComplexity, [this](int32)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetInt32Parameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalSplineComplexity"), PropertySet->PathComplexity);
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->VineThickness, [this](float)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				IvyActor->SetVineThickness(PropertySet->VineThickness);
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->MeshOffsetDistance, [this](float)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalOffsetDistance"), FVector(PropertySet->MeshOffsetDistance));
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->LeafSpawnSpacing, [this](float)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalLeafDensity"), PropertySet->LeafSpawnSpacing);
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->LeafScaleRange, [this](FVector2D)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinScale"), FVector(PropertySet->LeafScaleRange.X));
				UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxScale"), FVector(PropertySet->LeafScaleRange.Y));
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->bUseRandomLeafRotation, [this](bool)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetRotatorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinRotation"), PropertySet->bUseRandomLeafRotation ? PropertySet->MinRandomRotation : FRotator());
				UPCGGraphParametersHelpers::SetRotatorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxRotation"), PropertySet->bUseRandomLeafRotation ? PropertySet->MaxRandomRotation : FRotator());
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->MinRandomRotation, [this](FRotator)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetRotatorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinRotation"), PropertySet->MinRandomRotation);
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->MaxRandomRotation, [this](FRotator)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetRotatorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxRotation"), PropertySet->MaxRandomRotation);
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->bUseRandomLeafOffset, [this](bool)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinOffset"), PropertySet->bUseRandomLeafOffset ? PropertySet->MinRandomOffset : FVector::Zero());
				UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxOffset"), PropertySet->bUseRandomLeafOffset ? PropertySet->MaxRandomOffset : FVector::Zero());
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->MinRandomOffset, [this](FVector)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinOffset"), PropertySet->MinRandomOffset);
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->MaxRandomOffset, [this](FVector)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinOffset"), PropertySet->MaxRandomOffset);
				IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
}

void UIvyCreator_PCG::OnHitByClick_Implementation(FInputDeviceRay ClickPos,
                                                          const FScriptableToolModifierStates& Modifiers)
{
	Super::OnHitByClick_Implementation(ClickPos, Modifiers);

	if (!PropertySet || PropertySet && !PropertySet->MeshData)
	{
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("IvyToolNoData", "You are trying to run this tool with no mesh data. Create a data asset (UHM_IvyToolMeshData) and pass that into the tool ."));
		return;
	}

	HighlightActors(ClickPos, Modifiers, true);
	
	
}

void UIvyCreator_PCG::OnHoverBegin_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers)
{
	HighlightActors(HoverPos, Modifiers, false);
}

FInputRayHit UIvyCreator_PCG::OnHoverHitTest_Implementation(FInputDeviceRay HoverPos,
	const FScriptableToolModifierStates& Modifiers)
{
	return Super::OnHoverHitTest_Implementation(HoverPos, Modifiers);
}

bool UIvyCreator_PCG::OnHoverUpdate_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers)
{
	return Super::OnHoverUpdate_Implementation(HoverPos, Modifiers);
}

void UIvyCreator_PCG::OnHoverEnd_Implementation(const FScriptableToolModifierStates& Modifiers)
{
	USelection* Selection = GEditor->GetSelectedActors();
	if (Selection->Num() > 0)
	{
		Selection->DeselectAll();
	}

	for (auto Item : SelectedActors)
	{
		GEditor->SelectActor((AActor*)Item.Key, true, false);
	}
}

void UIvyCreator_PCG::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);
}

void UIvyCreator_PCG::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);

	switch (ShutdownType) {
	case EToolShutdownType::Completed:
		UE_LOG(LogTemp, Warning, TEXT("Ivy Creator Tool Completed"));
		break;
	case EToolShutdownType::Accept:
		HandleAccept();
		UE_LOG(LogTemp, Warning, TEXT("Ivy Creator Tool Accepted"));
		break;
	case EToolShutdownType::Cancel:
		HandleCancel();
		UE_LOG(LogTemp, Warning, TEXT("Ivy Creator Tool Cancelled"));
		break;
	}

	USelection* Selection = GEditor->GetSelectedActors();
	if (Selection->Num() > 0)
	{
		Selection->DeselectAll();
	}
}

void UIvyCreator_PCG::UpdateAcceptWarnings(EAcceptWarning Warning)
{
	Super::UpdateAcceptWarnings(Warning);
}

bool UIvyCreator_PCG::CanAccept() const
{
	return PropertySet && SelectedActors.Num() > 0;
}

bool UIvyCreator_PCG::HasCancel() const
{
	return true;
}

// UIvyCreator_PCG_PropertySet


void UIvyCreator_PCG::HighlightSelectedActor(const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection, const FHitResult& HitResult)
{
	
		// If I'm just clicking on an object, then I want to select it
		// If its already selected, then I want to deselect it
		// Change its color to yellow.

	const AActor* HitActor = HitResult.GetActor();

	TArray<AActor*> OutKeys;
	SelectedActors.GetKeys(OutKeys);

	// this instance we are on is in the selected actors list check if this actor we are selecting is in the list
	bool bHasSolvedSelection = false;
	if (SelectedActors.Contains(HitActor) && SelectedActors[HitActor].Selected)
	{
		GEditor->SelectActor(HitResult.GetActor(), false, false);
		
		// Remove this actor from the map and destroy its inworld actor
		SelectedActors[HitActor].Selected->Destroy();
		SelectedActors[HitActor].Selected = nullptr;
		bHasSolvedSelection = true;
	}
	else
	{
		if (!SelectedActors.Contains(HitActor) || SelectedActors.Contains(HitActor) && !SelectedActors[HitActor].Selected)
		{
			GEditor->SelectActor(HitResult.GetActor(), true, false);
			FObjectSelection NewSelection;
			AActor* NewIvyActor = SpawnNewIvyWorldActor(HitActor);

			check(NewIvyActor)
			

			NewSelection.Selected = NewIvyActor;
			
			
			

			SelectedActors.Add(HitResult.GetActor(), NewSelection);
		
			bHasSolvedSelection = true;
		}
	}
	
	
	for (auto Highlight : SelectedActors)
	{
		GEditor->SelectActor((AActor*)Highlight.Value.Selected, true, false);
	}
	
}


void UIvyCreator_PCG::HandleAccept()
{
	// Destroy all of the static mesh actors that We've added ivy actors to.
	UEditorActorSubsystem* ActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	for (auto Item : SelectedActors)
	{
		if (Item.Value.Selected)
		{
			ActorSubsystem->DestroyActor(Item.Key);	
		}
	}
}

void UIvyCreator_PCG::HandleCancel()
{
	UEditorActorSubsystem* ActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	for (auto Item : SelectedActors)
	{
		if (Cast<AActor>(Item.Value.Selected))
		{
			ActorSubsystem->DestroyActor(Cast<AActor>(Item.Value.Selected));
		}
	}

	SelectedActors.Empty();
}

AActor* UIvyCreator_PCG::SpawnNewIvyWorldActor(const AActor* ActorToSpawnOn)
{
	const AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(ActorToSpawnOn);
	if (!StaticMeshActor)
	{
		
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("IvyToolWrongGeo", "You've Selected an Actor that is not a Static Mesh Actor. Currently this tool only supports Static Mesh Actors."));
		return nullptr;
	}

	if (GetHandyManAPI())
	{
		if (!GetHandyManAPI()->GetPCGActorClass(FName(ToolName.ToString())))
		{
			return nullptr;
			
		}
		APCG_IvyActor* IvyActor = GetWorld()->SpawnActorDeferred<APCG_IvyActor>(GetHandyManAPI()->GetPCGActorClass(FName(ToolName.ToString())), FTransform::Identity);
		IvyActor->SetDisplayMesh(StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh());
		IvyActor->SetVineMaterial(PropertySet->MeshData->VineMaterial);
		IvyActor->SetDisplayMeshTransform(FTransform( FQuat(),FVector(), StaticMeshActor->GetActorScale3D()));
		IvyActor->SetVineThickness(PropertySet->VineThickness);
		IvyActor->TransferMeshMaterials(StaticMeshActor->GetStaticMeshComponent()->GetMaterials());
		

		const FVector& Location = StaticMeshActor->GetActorLocation();
		const FRotator& Rotation = StaticMeshActor->GetActorRotation();
		const FVector& Scale = FVector(1, 1, 1);
		IvyActor->FinishSpawning(FTransform(Rotation, Location, Scale));

		if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {return nullptr;}
		UPCGGraphParametersHelpers::SetBoolParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalVoxelizeMesh"), PropertySet->bVoxelizeMesh);
		UPCGGraphParametersHelpers::SetBoolParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalDebug"), PropertySet->bDebugMeshPoints);
		UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalVoxelSize"), PropertySet->VoxelSize);
		UPCGGraphParametersHelpers::SetInt32Parameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalSeed"), PropertySet->RandomSeed);
		UPCGGraphParametersHelpers::SetInt32Parameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalSplineCount"), PropertySet->SplineCount);
		UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalStartDistance"), PropertySet->StartingPointsHeightRatio);
		UPCGGraphParametersHelpers::SetInt32Parameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalSplineComplexity"), PropertySet->PathComplexity);
		UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalOffsetDistance"), FVector(0, 0, PropertySet->MeshOffsetDistance));
		UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalLeafDensity"), PropertySet->LeafSpawnSpacing);
		UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinScale"), FVector(PropertySet->LeafScaleRange.X));
		UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxScale"), FVector(PropertySet->LeafScaleRange.Y));
		UPCGGraphParametersHelpers::SetRotatorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinRotation"), PropertySet->bUseRandomLeafRotation ? PropertySet->MinRandomRotation : FRotator());
		UPCGGraphParametersHelpers::SetRotatorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxRotation"), PropertySet->bUseRandomLeafRotation ? PropertySet->MaxRandomRotation : FRotator());
		UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinOffset"), PropertySet->bUseRandomLeafOffset ? PropertySet->MinRandomOffset : FVector::Zero());
		UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxOffset"), PropertySet->bUseRandomLeafOffset ? PropertySet->MaxRandomOffset : FVector::Zero());
		UPCGGraphParametersHelpers::SetObjectParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMeshData"), PropertySet->MeshData);
		IvyActor->RerunConstructionScripts();
		IvyActor->GetPCGComponent()->GenerateLocal(true);
		IvyActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
		return IvyActor;
	}
	
	
	
	return nullptr;
	
}

void UIvyCreator_PCG::HighlightActors(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection)
{
	if (PropertySet)
	{
		const TEnumAsByte<ECollisionChannel> ChannelToTrace = TEnumAsByte<ECollisionChannel>(ECC_WorldStatic);
		// Do something with the property set
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, ClickPos.WorldRay.Origin, ClickPos.WorldRay.Origin + ClickPos.WorldRay.Direction * 10000.0f, ChannelToTrace))
		{
			if (!HitResult.GetComponent() || !HitResult.GetComponent()->IsVisible() || HitResult.GetComponent()->IsA(AVolume::StaticClass()) || HitResult.GetComponent()->IsA(APartitionActor::StaticClass()))
			{
				return;
			}
			
			HighlightSelectedActor(Modifiers, bShouldEditSelection, HitResult);
		}
	}
	
}

UIvyCreator_PCG_PropertySet::UIvyCreator_PCG_PropertySet()
{
}

#undef LOCTEXT_NAMESPACE