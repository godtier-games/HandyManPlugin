// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/PCG/IvyTool/IvyCreator_PCG.h"

#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Selection.h"
#include "ActorPartition/PartitionActor.h"
#include "Helpers/PCGGraphParametersHelpers.h"
#include "ToolSet/Core/HM_IvyToolMeshData.h"
#include "ToolSet/HandyManTools/PCG/IvyTool/PCG_IvyActor.h"

#define LOCTEXT_NAMESPACE "UIvyCreator_PCG"

UIvyCreator_PCG::UIvyCreator_PCG(): PropertySet(nullptr)
{
	ToolName = FText::FromString("Ivy Creator");
	ToolTooltip = FText::FromString("Procedurally Place Ivy on selected Static Meshes");
	ToolCategory = FText::FromString("Houdini | World Building");
	ToolLongName = FText::FromString("Ivy Creator");
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
				UPCGGraphParametersHelpers::SetObjectParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMeshData"), PropertySet->MeshData);
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
				UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalStartingPointsHeightRatio"), PropertySet->StartingPointsHeightRatio);
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
				UPCGGraphParametersHelpers::SetInt32Parameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalPathComplexity"), PropertySet->PathComplexity);
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
				UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalVineThickness"), PropertySet->VineThickness);
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
				UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMeshOffsetDistance"), PropertySet->MeshOffsetDistance);
			}
		}
	});
	PropertySet->WatchProperty(PropertySet->LeafDensity, [this](float)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalLeafDensity"), PropertySet->LeafDensity);
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
				UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalLeafScaleMin"), FVector(PropertySet->LeafScaleRange.X));
				UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalLeafScaleMax"), FVector(PropertySet->LeafScaleRange.Y));
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
				UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalLeafScaleMin"), FVector(PropertySet->LeafScaleRange.X));
				UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalLeafScaleMax"), FVector(PropertySet->LeafScaleRange.Y));
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
			}
		}
	});
}

void UIvyCreator_PCG::OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
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
			NewSelection.Selected = SpawnNewIvyWorldActor(HitActor);

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
	for (auto Item : SelectedActors)
	{
		if (Item.Value.Selected)
		{
			Item.Key->Destroy();	
		}
	}
}

void UIvyCreator_PCG::HandleCancel()
{
	for (auto Item : SelectedActors)
	{
		if (Cast<AActor>(Item.Value.Selected))
		{
			Cast<AActor>(Item.Value.Selected)->Destroy();
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

	APCG_IvyActor* IvyActor = GetWorld()->SpawnActor<APCG_IvyActor>(APCG_IvyActor::StaticClass(), StaticMeshActor->GetActorLocation(), StaticMeshActor->GetActorRotation());
	IvyActor->SetDisplayMesh(StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh());
	IvyActor->SetDisplayMeshTransform(FTransform( FQuat(),FVector(), StaticMeshActor->GetActorScale3D()));
	IvyActor->SetVineThickness(PropertySet->VineThickness);
	
	if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {return IvyActor;}
	UPCGGraphParametersHelpers::SetInt32Parameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalSeed"), PropertySet->RandomSeed);
	UPCGGraphParametersHelpers::SetInt32Parameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalSplineCount"), PropertySet->SplineCount);
	UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalStartDistance"), PropertySet->StartingPointsHeightRatio);
	UPCGGraphParametersHelpers::SetInt32Parameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalPathComplexity"), PropertySet->PathComplexity);
	UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalOffsetDistance"), FVector(0, 0, PropertySet->MeshOffsetDistance));
	UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalLeafDensity"), PropertySet->LeafDensity);
	UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalLeafScaleMin"), FVector(PropertySet->LeafScaleRange.X));
	UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalLeafScaleMax"), FVector(PropertySet->LeafScaleRange.Y));
	UPCGGraphParametersHelpers::SetRotatorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinRotation"), PropertySet->bUseRandomLeafRotation ? PropertySet->MinRandomRotation : FRotator());
	UPCGGraphParametersHelpers::SetRotatorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxRotation"), PropertySet->bUseRandomLeafRotation ? PropertySet->MaxRandomRotation : FRotator());
	UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMinOffset"), PropertySet->bUseRandomLeafOffset ? PropertySet->MinRandomOffset : FVector::Zero());
	UPCGGraphParametersHelpers::SetVectorParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMaxOffset"), PropertySet->bUseRandomLeafOffset ? PropertySet->MaxRandomOffset : FVector::Zero());
	UPCGGraphParametersHelpers::SetObjectParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalMeshData"), PropertySet->MeshData);
	
	return IvyActor;
	
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