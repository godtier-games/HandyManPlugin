// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/PCG/IvyTool/IvyCreator_PCG.h"

#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Selection.h"
#include "Helpers/PCGGraphParametersHelpers.h"
#include "ToolSet/HandyManTools/PCG/IvyTool/PCG_IvyActor.h"

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
	PropertySet->WatchProperty(PropertySet->EndingPointsHeightRatio, [this](float)
	{
		for (auto Item : SelectedActors)
		{
			if (APCG_IvyActor* IvyActor = Cast<APCG_IvyActor>(Item.Value.Selected))
			{
				if(!IvyActor->GetPCGComponent() || !IvyActor->GetPCGComponent()->GetGraphInstance()) {continue;}
				UPCGGraphParametersHelpers::SetFloatParameter(IvyActor->GetPCGComponent()->GetGraphInstance(), FName("GlobalEndingPointsHeightRatio"), PropertySet->EndingPointsHeightRatio);
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
	if (Modifiers.bShiftDown)
	{
		// I want to add this to a group

		// If its the first item of the group, then create a new group

		// Change highlight color to green g = group

		if (CurrentInstance != nullptr)
		{
			auto FoundObjects = SelectedActors.FindRef(CurrentInstance);
			if (SelectedActors.Contains(CurrentInstance) && !SelectedActors[CurrentInstance].Selected.Contains(HitResult.GetActor()))
			{
				GEditor->SelectActor(HitResult.GetActor(), true, false);
				if(bShouldEditSelection) SelectedActors[CurrentInstance].Selected.Add(HitResult.GetActor());

				// Remove from duplicate selections
				for (auto& Item : SelectedActors)
				{
					if (!Item.Value.Selected.Contains(HitResult.GetActor()) || Item.Key == CurrentInstance) continue;
					
					if(bShouldEditSelection) Item.Value.Selected.Remove(HitResult.GetActor());
					GEditor->SelectActor(HitResult.GetActor(), false, false);
				}
			}
		}
		else
		{
			for (auto& FindEmpty : SelectedActors)
			{
				if (FindEmpty.Value.Selected.Num() == 0)
				{
					FindEmpty.Value.Selected.Add(HitResult.GetActor());
					CurrentInstance = FindEmpty.Key;
					GEditor->SelectActor(HitResult.GetActor(), true, false);
				}
				else
				{
					CurrentInstance = SpawnHDAInstance();
					SelectedActors.Add(CurrentInstance, FObjectSelections(HitResult.GetActor()));
					GEditor->SelectActor(HitResult.GetActor(), true, false);
				}
			}
		}
	}
	else if (Modifiers.bCtrlDown)
	{
		// I want to remove this from a group
		// If its not apart of a group then this will just delect the item
		// if its apart of a group it will remove from group but remain selected (add new instance and add this to that instances selected objects)
		// Change highlight color to blue

		if (CurrentInstance != nullptr)
		{
			bool bHasSolvedSelection = false;
			if (SelectedActors.Contains(CurrentInstance) && SelectedActors[CurrentInstance].Selected.Contains(HitResult.GetActor()))
			{
				if(bShouldEditSelection)
				{
					SelectedActors[CurrentInstance].Selected.Remove(HitResult.GetActor());
					
					for (auto& FindEmpty : SelectedActors)
					{
						if (FindEmpty.Value.Selected.Num() == 0)
						{
							FindEmpty.Value.Selected.Add(HitResult.GetActor());
							CurrentInstance = FindEmpty.Key;
							bHasSolvedSelection = true;
						}
					}

					if(!bHasSolvedSelection)
					{
						SelectedActors[CurrentInstance].Selected.Remove(HitResult.GetActor());
						CurrentInstance = SpawnHDAInstance();
						SelectedActors.Add(CurrentInstance, FObjectSelections(HitResult.GetActor()));
					}
					
				}
				
			}
			else
			{
				for (auto& Item : SelectedActors)
				{
					if (!Item.Value.Selected.Contains(HitResult.GetActor())) continue;
					if(bShouldEditSelection && Item.Value.Selected.Num() > 1)
					{
						for (auto& FindEmpty : SelectedActors)
						{
							if (FindEmpty.Value.Selected.Num() == 0)
							{
								Item.Value.Selected.Remove(HitResult.GetActor());
								FindEmpty.Value.Selected.Add(HitResult.GetActor());
								CurrentInstance = FindEmpty.Key;
								bHasSolvedSelection = true;
							}
						}

						if (!bHasSolvedSelection)
						{
						
							Item.Value.Selected.Remove(HitResult.GetActor());
							CurrentInstance = SpawnHDAInstance();
							SelectedActors.Add(CurrentInstance, FObjectSelections(HitResult.GetActor()));
							
						}

						

						
					}
					
				}
			}

			if (!bHasSolvedSelection)
			{
				/*CurrentInstance = SpawnHDAInstance();
				SelectedActors.Add(CurrentInstance, FObjectSelection(HitResult.GetActor()));*/
			}
		}
	}
	else
	{
		// If I'm just clicking on an object, then I want to select it
		// If its already selected, then I want to deselect it
		// Change its color to yellow.

		TArray<UHoudiniPublicAPIAssetWrapper*> OutKeys;
		SelectedActors.GetKeys(OutKeys);

		// this instance we are on is in the selected actors list check if this actor we are selecting is in the list
		if (CurrentInstance != nullptr)
		{
			bool bHasSolvedSelection = false;
			if (SelectedActors.Contains(CurrentInstance) && SelectedActors[CurrentInstance].Selected.Contains(HitResult.GetActor()))
			{
				GEditor->SelectActor(HitResult.GetActor(), false, false);
				if(bShouldEditSelection) SelectedActors[CurrentInstance].Selected.Remove(HitResult.GetActor());
				bHasSolvedSelection = true;
			}
			else
			{
				for (auto& Item : SelectedActors)
				{
					if (!Item.Value.Selected.Contains(HitResult.GetActor())) continue;
					
					GEditor->SelectActor(HitResult.GetActor(), false, false);
					if(bShouldEditSelection) Item.Value.Selected.Remove(HitResult.GetActor());
					CurrentInstance = Item.Key;
					bHasSolvedSelection = true;
				}
			}

			if (!bHasSolvedSelection)
			{
				for (auto& FindEmpty : SelectedActors)
				{
					if (FindEmpty.Value.Selected.Num() == 0)
					{
						FindEmpty.Value.Selected.Add(HitResult.GetActor());
						CurrentInstance = FindEmpty.Key;
						GEditor->SelectActor(HitResult.GetActor(), true, false);
						bHasSolvedSelection = true;
					}
				}

				if (!bHasSolvedSelection)
				{
					CurrentInstance = SpawnHDAInstance();
					SelectedActors.Add(CurrentInstance, FObjectSelections(HitResult.GetActor()));
					GEditor->SelectActor(HitResult.GetActor(), true, false);
				}
				
				
			}
		}
		else
		{
			CurrentInstance = SpawnHDAInstance();
			SelectedActors.Add(CurrentInstance, FObjectSelections(HitResult.GetActor()));
			GEditor->SelectActor(HitResult.GetActor(), true, false);
		}
	}

	for (auto Highlight : SelectedActors)
	{
		for (int i = 0; i < Highlight.Value.Selected.Num(); i++)
		{
			if (Highlight.Value.Selected[i] == nullptr)
			{
				Highlight.Value.Selected.RemoveAt(i);
				continue;
			}
			GEditor->SelectActor((AActor*)Highlight.Value.Selected[i], true, false);
		};
	}
	
}

void UIvyCreator_PCG::HandleAccept()
{
	for (auto Item : SelectedActors)
	{
		if (Item.Key)
		{
			if(Item.Value.Selected.Num() == 0) {Item.Key->DeleteInstantiatedAsset(); continue;} 
			UHoudiniPublicAPIWorldInput* InputGeo = Cast<UHoudiniPublicAPIWorldInput>(Item.Key->CreateEmptyInput(UHoudiniPublicAPIWorldInput::StaticClass()));

			TArray<UObject*> InputObjects;
			for (auto Object : Item.Value.Selected)
			{
				InputObjects.Add(Object);
			}
			
			InputGeo->SetInputObjects(InputObjects);
			Item.Key->SetInputAtIndex(0, InputGeo);
			Item.Key->Recook();
			Item.Key->SetAutoCookingEnabled(true);
		}
		

		
	}
}

void UIvyCreator_PCG::HandleCancel()
{
	for (auto Item : SelectedActors)
	{
		if (Item.Key)
		{
			Item.Key->DeleteInstantiatedAsset();
		}
	}

	SelectedActors.Empty();
}

void UIvyCreator_PCG::HighlightActors(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection)
{
	if (PropertySet)
	{
		const TEnumAsByte<ECollisionChannel> ChannelToTrace = PropertySet->bUseCustomTraceChannel ? PropertySet->CustomTraceChannel : TEnumAsByte<ECollisionChannel>(ECC_WorldStatic);
		// Do something with the property set
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, ClickPos.WorldRay.Origin, ClickPos.WorldRay.Origin + ClickPos.WorldRay.Direction * 10000.0f, ChannelToTrace))
		{
			if (!HitResult.GetComponent() || !HitResult.GetComponent()->IsVisible() || HitResult.GetComponent()->IsA(AVolume::StaticClass()) || HitResult.GetComponent()->IsA(APartitionActor::StaticClass()))
			{
				return;
			}
			
			if (PropertySet->Filter.Num() > 0)
			{
				for (auto Item : PropertySet->Filter)
				{
					if (PropertySet->bExcludeFilter)
					{
						if (HitResult.GetActor()->IsA(Item)) {continue;}

						HighlightSelectedActor(Modifiers, bShouldEditSelection, HitResult);
					}
					else
					{
						if (!HitResult.GetActor()->IsA(Item)) {continue;}

						HighlightSelectedActor(Modifiers, bShouldEditSelection, HitResult);
					}
				}
			}
			else
			{
				HighlightSelectedActor(Modifiers, bShouldEditSelection, HitResult);
			}
		}
	}
	
}

UIvyCreator_PCG_PropertySet::UIvyCreator_PCG_PropertySet()
{
}

UHoudiniPublicAPIAssetWrapper* UIvyCreator_PCG::SpawnHDAInstance()
{
	UHoudiniPublicAPIAssetWrapper* returnObj = nullptr;
	
	if (PropertySet && GetHandyManAPI())
	{
		returnObj = GetHandyManAPI()->GetMutableHoudiniAPI()->InstantiateAsset
		(
				GetHandyManAPI()->GetHoudiniDigitalAsset(EHandyManToolName::IvyTool),
				FTransform::Identity,
				nullptr,
				nullptr,
				false,
				false, PropertySet->AssetSaveLocation.FilePath, PropertySet->BakeType, !PropertySet->bKeepProceduralAssets
		);
	}
	
	return returnObj;
}
