// Fill out your copyright notice in the Description page of Project Settings.


#include "ScatterGeometryTool.h"
#include "HandyManSettings.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Selection.h"
#include "ActorPartition/PartitionActor.h"
#include "Helpers/PCGGraphParametersHelpers.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "ToolSet/Core/HM_ScatterToolGeometryData.h"
#include "ToolSet/HandyManTools/PCG/ScatterMeshTool/Actor/PCG_ScatterMeshActor.h"

#define LOCTEXT_NAMESPACE "UScatterGeometryTool"

UScatterGeometryTool::UScatterGeometryTool(): PropertySet(nullptr)
{
	ToolName = FText::FromString("Mesh Scatter");
	ToolTooltip = FText::FromString("Scatter geometry on selected static mesh actors.");
	ToolCategory = FText::FromString("World Building");
	ToolLongName = FText::FromString("Scatter Geometry On Mesh");
}

void UScatterGeometryTool::Setup()
{
	Super::Setup();
	
	GEditor->GetSelectedActors()->DeselectAll();

	EToolsFrameworkOutcomePins PropertyCreationOutcome;
	PropertySet = Cast<UScatterGeometryTool_PropertySet>(AddPropertySetOfType(UScatterGeometryTool_PropertySet::StaticClass(), "Settings", PropertyCreationOutcome));

	PropertySet->WatchProperty(PropertySet->MeshData, [this](UHM_ScatterToolGeometryData*)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_ScatterMeshActor* ScatterActor = Cast<APCG_ScatterMeshActor>(Item.Value.Selected))
			{
				if(!ScatterActor->GetPCGComponent() || !ScatterActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				
				UPCGGraphParametersHelpers::SetObjectParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMeshData"), PropertySet->MeshData);
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->bVoxelizeMesh, [this](bool)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_ScatterMeshActor* ScatterActor = Cast<APCG_ScatterMeshActor>(Item.Value.Selected))
			{
				if(!ScatterActor->GetPCGComponent() || !ScatterActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetBoolParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalVoxelizeMesh"), PropertySet->bVoxelizeMesh);
				ScatterActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->VoxelSize, [this](float)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_ScatterMeshActor* ScatterActor = Cast<APCG_ScatterMeshActor>(Item.Value.Selected))
			{
				if(!ScatterActor->GetPCGComponent() || !ScatterActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetFloatParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalVoxelSize"), PropertySet->VoxelSize);
				ScatterActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->RandomSeed, [this](int32)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_ScatterMeshActor* ScatterActor = Cast<APCG_ScatterMeshActor>(Item.Value.Selected))
			{
				if(!ScatterActor->GetPCGComponent() || !ScatterActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetInt32Parameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalSeed"), PropertySet->RandomSeed);
				ScatterActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
			}
		}
	});
	
	PropertySet->WatchProperty(PropertySet->GeometryScaleRange, [this](FVector2D)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_ScatterMeshActor* ScatterActor = Cast<APCG_ScatterMeshActor>(Item.Value.Selected))
			{
				if(!ScatterActor->GetPCGComponent() || !ScatterActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetVectorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinScale"), FVector(PropertySet->GeometryScaleRange.X));
				UPCGGraphParametersHelpers::SetVectorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxScale"), FVector(PropertySet->GeometryScaleRange.Y));
				ScatterActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->bUseRandomRotation, [this](bool)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_ScatterMeshActor* ScatterActor = Cast<APCG_ScatterMeshActor>(Item.Value.Selected))
			{
				if(!ScatterActor->GetPCGComponent() || !ScatterActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetRotatorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinRotation"), PropertySet->bUseRandomRotation ? PropertySet->MinRandomRotation : FRotator());
				UPCGGraphParametersHelpers::SetRotatorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxRotation"), PropertySet->bUseRandomRotation ? PropertySet->MaxRandomRotation : FRotator());
				ScatterActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->MinRandomRotation, [this](FRotator)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_ScatterMeshActor* ScatterActor = Cast<APCG_ScatterMeshActor>(Item.Value.Selected))
			{
				if(!ScatterActor->GetPCGComponent() || !ScatterActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetRotatorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinRotation"), PropertySet->MinRandomRotation);
				ScatterActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->MaxRandomRotation, [this](FRotator)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_ScatterMeshActor* ScatterActor = Cast<APCG_ScatterMeshActor>(Item.Value.Selected))
			{
				if(!ScatterActor->GetPCGComponent() || !ScatterActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetRotatorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxRotation"), PropertySet->MaxRandomRotation);
				ScatterActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->bUseRandomOffset, [this](bool)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_ScatterMeshActor* ScatterActor = Cast<APCG_ScatterMeshActor>(Item.Value.Selected))
			{
				if(!ScatterActor->GetPCGComponent() || !ScatterActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetVectorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinOffset"), PropertySet->bUseRandomOffset ? PropertySet->MinRandomOffset : FVector::Zero());
				UPCGGraphParametersHelpers::SetVectorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxOffset"), PropertySet->bUseRandomOffset ? PropertySet->MaxRandomOffset : FVector::Zero());
				ScatterActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->MinRandomOffset, [this](FVector)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_ScatterMeshActor* ScatterActor = Cast<APCG_ScatterMeshActor>(Item.Value.Selected))
			{
				if(!ScatterActor->GetPCGComponent() || !ScatterActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetVectorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinOffset"), PropertySet->MinRandomOffset);
				ScatterActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
	PropertySet->WatchProperty(PropertySet->MaxRandomOffset, [this](FVector)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_ScatterMeshActor* ScatterActor = Cast<APCG_ScatterMeshActor>(Item.Value.Selected))
			{
				if(!ScatterActor->GetPCGComponent() || !ScatterActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetVectorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinOffset"), PropertySet->MaxRandomOffset);
				ScatterActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();

			}
		}
	});
}

void UScatterGeometryTool::OnHitByClick_Implementation(FInputDeviceRay ClickPos,
                                                               const FScriptableToolModifierStates& Modifiers)
{
	Super::OnHitByClick_Implementation(ClickPos, Modifiers);

	if (!PropertySet || PropertySet && !PropertySet->MeshData)
	{
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("ScatterGeometryToolNoData", "You are trying to run this tool with no mesh data. Create a data asset (UHM_ScatterToolGeometryData) and pass that into the tool ."));
		return;
	}

	HighlightActors(ClickPos, Modifiers, true);
	
	
}

void UScatterGeometryTool::OnHoverBegin_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers)
{
	HighlightActors(HoverPos, Modifiers, false);
}

FInputRayHit UScatterGeometryTool::OnHoverHitTest_Implementation(FInputDeviceRay HoverPos,
	const FScriptableToolModifierStates& Modifiers)
{
	return Super::OnHoverHitTest_Implementation(HoverPos, Modifiers);
}

bool UScatterGeometryTool::OnHoverUpdate_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers)
{
	return Super::OnHoverUpdate_Implementation(HoverPos, Modifiers);
}

void UScatterGeometryTool::OnHoverEnd_Implementation(const FScriptableToolModifierStates& Modifiers)
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

void UScatterGeometryTool::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);
}

void UScatterGeometryTool::Shutdown(EToolShutdownType ShutdownType)
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

void UScatterGeometryTool::UpdateAcceptWarnings(EAcceptWarning Warning)
{
	Super::UpdateAcceptWarnings(Warning);
}

bool UScatterGeometryTool::CanAccept() const
{
	return PropertySet && SelectedActors.Num() > 0;
}

bool UScatterGeometryTool::HasCancel() const
{
	return true;
}

// UScatterGeometryTool_PropertySet


void UScatterGeometryTool::HighlightSelectedActor(const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection, const FHitResult& HitResult)
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
			AActor* NewScatterActor = SpawnNewScatterWorldActor(HitActor);

			check(NewScatterActor)
			

			NewSelection.Selected = NewScatterActor;
			
			
			

			SelectedActors.Add(HitResult.GetActor(), NewSelection);
		
			bHasSolvedSelection = true;
		}
	}
	
	
	for (auto Highlight : SelectedActors)
	{
		GEditor->SelectActor((AActor*)Highlight.Value.Selected, true, false);
	}
	
}


void UScatterGeometryTool::HandleAccept()
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

void UScatterGeometryTool::HandleCancel()
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

AActor* UScatterGeometryTool::SpawnNewScatterWorldActor(const AActor* ActorToSpawnOn)
{
	const AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(ActorToSpawnOn);
	if (!StaticMeshActor)
	{
		
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("ScatterGeometryToolWrongGeo", "You've Selected an Actor that is not a Static Mesh Actor. Currently this tool only supports Static Mesh Actors."));
		return nullptr;
	}

	if (GetHandyManAPI())
	{
		if (!GetHandyManAPI()->GetPCGActorClass(FName(ToolName.ToString())))
		{
			return nullptr;
			
		}
		APCG_ScatterMeshActor* ScatterActor = GetWorld()->SpawnActorDeferred<APCG_ScatterMeshActor>(GetHandyManAPI()->GetPCGActorClass(FName(ToolName.ToString())), FTransform::Identity);
		ScatterActor->SetDisplayMesh(StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh());
		ScatterActor->SetDisplayMeshTransform(FTransform( FQuat(),FVector(), StaticMeshActor->GetActorScale3D()));
		ScatterActor->TransferMeshMaterials(StaticMeshActor->GetStaticMeshComponent()->GetMaterials());
		

		const FVector& Location = StaticMeshActor->GetActorLocation();
		const FRotator& Rotation = StaticMeshActor->GetActorRotation();
		const FVector& Scale = FVector(1, 1, 1);
		ScatterActor->FinishSpawning(FTransform(Rotation, Location, Scale));

		if(!ScatterActor->GetPCGComponent() || !ScatterActor->GetPCGComponent()->GetGraphInstance()) {return nullptr;}
		UPCGGraphParametersHelpers::SetBoolParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalVoxelizeMesh"), PropertySet->bVoxelizeMesh);
		UPCGGraphParametersHelpers::SetFloatParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalVoxelSize"), PropertySet->VoxelSize);
		UPCGGraphParametersHelpers::SetInt32Parameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalSeed"), PropertySet->RandomSeed);
		UPCGGraphParametersHelpers::SetVectorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinScale"), FVector(PropertySet->GeometryScaleRange.X));
		UPCGGraphParametersHelpers::SetVectorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxScale"), FVector(PropertySet->GeometryScaleRange.Y));
		UPCGGraphParametersHelpers::SetRotatorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinRotation"), PropertySet->bUseRandomRotation ? PropertySet->MinRandomRotation : FRotator());
		UPCGGraphParametersHelpers::SetRotatorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxRotation"), PropertySet->bUseRandomRotation ? PropertySet->MaxRandomRotation : FRotator());
		UPCGGraphParametersHelpers::SetVectorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinOffset"), PropertySet->bUseRandomOffset ? PropertySet->MinRandomOffset : FVector::Zero());
		UPCGGraphParametersHelpers::SetVectorParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxOffset"), PropertySet->bUseRandomOffset ? PropertySet->MaxRandomOffset : FVector::Zero());
		UPCGGraphParametersHelpers::SetObjectParameter(ScatterActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMeshData"), PropertySet->MeshData);
		ScatterActor->RerunConstructionScripts();
		ScatterActor->GetPCGComponent()->GenerateLocal(true);
		ScatterActor->GetPCGComponent()->NotifyPropertiesChangedFromBlueprint();
		return ScatterActor;
	}
	
	
	
	return nullptr;
	
}

void UScatterGeometryTool::HighlightActors(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection)
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

UScatterGeometryTool_PropertySet::UScatterGeometryTool_PropertySet()
{
}

#undef LOCTEXT_NAMESPACE