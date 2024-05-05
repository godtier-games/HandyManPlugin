// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/Houdini/WorldBuilding/ProceduralTool_Ivy/IvyCreator.h"
#include "Selection.h"
#include "HoudiniPublicAPI.h"
#include "HoudiniPublicAPIAssetWrapper.h"
#include "Engine/TriggerVolume.h"


UIvyCreator::UIvyCreator():PropertySet(nullptr)
{
	ToolName = FText::FromString("Ivy Creator");
	ToolTooltip = FText::FromString("Procedurally Place Ivy on selected Static Meshes");
	ToolCategory = FText::FromString("Houdini | World Building");
	ToolLongName = FText::FromString("Ivy Creator");
}

void UIvyCreator::Setup()
{
	Super::Setup();

	EToolsFrameworkOutcomePins PropertyCreationOutcome;
	PropertySet = Cast<UIvyCreator_PropertySet>(AddPropertySetOfType(UIvyCreator_PropertySet::StaticClass(), "Settings", PropertyCreationOutcome));

	if (GetHoudiniAPI())
	{
		if (!GetHoudiniAPI()->IsSessionValid())
		{
			GetMutableAPI()->CreateSession();
		}
	}

	
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
	if (SelectedActors.Contains(HitResult.GetActor()))
	{
		if (SelectedActors.FindRef(HitResult.GetActor()))
		{
			if (bShouldEditSelection) {SelectedActors.FindRef(HitResult.GetActor())->DeleteInstantiatedAsset(); SelectedActors.Remove(HitResult.GetActor());}
		}
		else
		{
			if (bShouldEditSelection) SelectedActors.Add(HitResult.GetActor(), SpawnHDAInstance());
		}
	}
	else
	{
		if (bShouldEditSelection) SelectedActors.Add(HitResult.GetActor(), SpawnHDAInstance());
	}

	for (auto Item : SelectedActors)
	{
		if (Item.Key && Item.Value)
		{
			GEditor->SelectActor((AActor*)Item.Key, true, false);
		}
	}
}

void UIvyCreator::HandleAccept()
{
	for (auto Item : SelectedActors)
	{
		if (Item.Key && Item.Value)
		{
			UHoudiniPublicAPIWorldInput* InputGeo = Cast<UHoudiniPublicAPIWorldInput>(Item.Value->CreateEmptyInput(UHoudiniPublicAPIWorldInput::StaticClass()));

			TArray<UObject*> InputObjects {Item.Key};
			InputGeo->SetInputObjects(InputObjects);
			Item.Value->SetInputAtIndex(0, InputGeo);
			Item.Value->Recook();
			Item.Value->SetAutoCookingEnabled(true);
		}
	}
	
}

void UIvyCreator::HighlightActors(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection)
{
	if (PropertySet)
	{
		const TEnumAsByte<ECollisionChannel> ChannelToTrace = PropertySet->bUseCustomTraceChannel ? PropertySet->CustomTraceChannel : TEnumAsByte<ECollisionChannel>(ECC_Visibility);
		// Do something with the property set
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, ClickPos.WorldRay.Origin, ClickPos.WorldRay.Direction * 10000.0f, ChannelToTrace))
		{
			if (!HitResult.GetComponent() || !HitResult.GetComponent()->IsVisible() || HitResult.GetComponent()->IsA(AVolume::StaticClass()))
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
	if (PropertySet)
	{
		returnObj = GetMutableAPI()->InstantiateAsset(
				GetHoudiniDigitalAsset(),
				FTransform::Identity,
nullptr,
nullptr,
false,
false, PropertySet->AssetSaveLocation.FilePath, PropertySet->BakeType, !PropertySet->bKeepProceduralAssets);
		
	}
	
	return returnObj;
}
