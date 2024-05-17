// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/Houdini/WorldBuilding/ProceduralTool_Ivy/IvyCreator.h"

#include "HandyManSettings.h"
#include "Selection.h"
#include "HoudiniPublicAPI.h"
#include "HoudiniPublicAPIAssetWrapper.h"
#include "Landscape.h"
#include "Engine/TriggerVolume.h"
#include "ToolSet/Core/HandyManSubsystem.h"


UIvyCreator::UIvyCreator(): PropertySet(nullptr), CurrentInstance(nullptr)
{
	ToolName = FText::FromString("Ivy Creator");
	ToolTooltip = FText::FromString("Procedurally Place Ivy on selected Static Meshes");
	ToolCategory = FText::FromString("Houdini | World Building");
	ToolLongName = FText::FromString("Ivy Creator");
}

void UIvyCreator::Setup()
{
	Super::Setup();

	if (GetHandyManAPI())
	{
		GetHandyManAPI()->InitializeHoudiniApi();
		
		if (!GetHandyManAPI()->GetMutableHoudiniAPI()->IsSessionValid())
		{
			GetHandyManAPI()->GetMutableHoudiniAPI()->CreateSession();
		}
	}

	GEditor->GetSelectedActors()->DeselectAll();

	EToolsFrameworkOutcomePins PropertyCreationOutcome;
	PropertySet = Cast<UIvyCreator_PropertySet>(AddPropertySetOfType(UIvyCreator_PropertySet::StaticClass(), "Settings", PropertyCreationOutcome));
	
}

void UIvyCreator::OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	Super::OnHitByClick_Implementation(ClickPos, Modifiers);

	HighlightActors(ClickPos, Modifiers, true);
	
	
}

void UIvyCreator::OnHoverBegin_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers)
{
	HighlightActors(HoverPos, Modifiers, false);
}

FInputRayHit UIvyCreator::OnHoverHitTest_Implementation(FInputDeviceRay HoverPos,
	const FScriptableToolModifierStates& Modifiers)
{
	return Super::OnHoverHitTest_Implementation(HoverPos, Modifiers);
}

bool UIvyCreator::OnHoverUpdate_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers)
{
	return Super::OnHoverUpdate_Implementation(HoverPos, Modifiers);
}

void UIvyCreator::OnHoverEnd_Implementation(const FScriptableToolModifierStates& Modifiers)
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

void UIvyCreator::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);
}

void UIvyCreator::Shutdown(EToolShutdownType ShutdownType)
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

void UIvyCreator::UpdateAcceptWarnings(EAcceptWarning Warning)
{
	Super::UpdateAcceptWarnings(Warning);
}

bool UIvyCreator::CanAccept() const
{
	return PropertySet && SelectedActors.Num() > 0;
}

bool UIvyCreator::HasCancel() const
{
	return true;
}

// UIvyCreator_PropertySet


void UIvyCreator::HighlightSelectedActor(const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection, const FHitResult& HitResult)
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
					SelectedActors.Add(CurrentInstance, FObjectSelection(HitResult.GetActor()));
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
						SelectedActors.Add(CurrentInstance, FObjectSelection(HitResult.GetActor()));
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
							SelectedActors.Add(CurrentInstance, FObjectSelection(HitResult.GetActor()));
							
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
					SelectedActors.Add(CurrentInstance, FObjectSelection(HitResult.GetActor()));
					GEditor->SelectActor(HitResult.GetActor(), true, false);
				}
				
				
			}
		}
		else
		{
			CurrentInstance = SpawnHDAInstance();
			SelectedActors.Add(CurrentInstance, FObjectSelection(HitResult.GetActor()));
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

void UIvyCreator::HandleAccept()
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

void UIvyCreator::HandleCancel()
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

void UIvyCreator::HighlightActors(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection)
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

UIvyCreator_PropertySet::UIvyCreator_PropertySet()
{
}

UHoudiniPublicAPIAssetWrapper* UIvyCreator::SpawnHDAInstance()
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
