// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/Houdini/WorldBuilding/DrapeTool_SingleClick/ClothSimTool_Drape.h"

#include "HandyManSettings.h"
#include "HoudiniPublicAPI.h"
#include "HoudiniPublicAPIAssetWrapper.h"
#include "Selection.h"
#include "Engine/StaticMeshActor.h"

void UClothSimToolActions::PostAction(const EDrapeToolStage_SingleClick Action)
{
	if (ParentTool.IsValid())
	{
		ParentTool->RequestAction(Action);
	}
}


UClothSimTool_Drape::UClothSimTool_Drape(): Settings(nullptr), Actions(nullptr), CurrentInstance(nullptr)
{
	ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
}

void UClothSimTool_Drape::RequestAction(const EDrapeToolStage_SingleClick Action)
{
	switch (Action)
	{
		case EDrapeToolStage_SingleClick::Beginning:
			break;
		case EDrapeToolStage_SingleClick::Collision:
			break;
		case EDrapeToolStage_SingleClick::Drape:
			break;
		case EDrapeToolStage_SingleClick::Pin:
			break;
		case EDrapeToolStage_SingleClick::Finish:
			break;
		case EDrapeToolStage_SingleClick::EditSelection:
			break;
		case EDrapeToolStage_SingleClick::EditGeometry:
			break;
		case EDrapeToolStage_SingleClick::Simulate:
			{
				if (!Settings) break;

				if (!CurrentInstance)
				{
					FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, FText::FromString("Internal Error: CurrentInstance not set. Exit The Tool & Try Again"));
					break;
				}
			

				if (!Settings->ClothGeometry)
				{
					FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, FText::FromString("Cloth Geometry not set"));
					break;
				}

				UHoudiniPublicAPIWorldInput* InputGeo_Cloth = Cast<UHoudiniPublicAPIWorldInput>(CurrentInstance->CreateEmptyInput(UHoudiniPublicAPIWorldInput::StaticClass()));
				InputGeo_Cloth->SetInputObjects({reinterpret_cast<UObject*>(Settings->ClothGeometry)});
				CurrentInstance->SetInputAtIndex(0, InputGeo_Cloth);

				if (Settings->PinGeometry.Num() > 0)
				{
					TArray<UObject*> InputObjects;
					UHoudiniPublicAPIWorldInput* InputGeo_Pin = Cast<UHoudiniPublicAPIWorldInput>(CurrentInstance->CreateEmptyInput(UHoudiniPublicAPIWorldInput::StaticClass()));
					for (const auto Item : Settings->PinGeometry)
					{
						InputObjects.Add(Cast<UObject>(Item));	
					}
					InputGeo_Pin->SetInputObjects(InputObjects);
					CurrentInstance->SetInputAtIndex(1, InputGeo_Pin);
				}

				if (Settings->CollisionGeometry.Num() > 0)
				{
					TArray<UObject*> InputObjects;
					UHoudiniPublicAPIWorldInput* InputGeo_Collision = Cast<UHoudiniPublicAPIWorldInput>(CurrentInstance->CreateEmptyInput(UHoudiniPublicAPIWorldInput::StaticClass()));
					for (const auto Item : Settings->CollisionGeometry)
					{
						InputObjects.Add(Cast<UObject>(Item));	
					}
					InputGeo_Collision->SetInputObjects(InputObjects);
					CurrentInstance->SetInputAtIndex(2, InputGeo_Collision);
				}

				Settings->ClothGeometry->SetActorHiddenInGame(!Settings->bHideInputClothGeometry);

				CurrentInstance->Recook();
				CurrentInstance->TriggerButtonParameter("resimulate");

				bCanAcceptTool = true;

				break;
			}
	}
}

void UClothSimTool_Drape::StartWatchingProperties()
{
	Settings->WatchProperty(Settings->ClothGeometry, [&](AStaticMeshActor*)
	{
		GEditor->GetSelectedActors()->DeselectAll();
		SelectedActors.Add(EDrapeToolStage_SingleClick::Drape, FObjectSelection(Settings->ClothGeometry));
		for (auto Item : SelectedActors)
		{
			for (auto ObjectItem : Item.Value.Selected)
			{
				if(!ObjectItem) { continue; }
				GEditor->SelectActor((AActor*)ObjectItem, true, false);
			}
		}
	});

	Settings->WatchProperty(Settings->CollisionGeometry, [&](TArray<AStaticMeshActor*>)
	{
		GEditor->GetSelectedActors()->DeselectAll();
		SelectedActors.Add(EDrapeToolStage_SingleClick::Collision, FObjectSelection(Settings->CollisionGeometry));
		for (auto Item : SelectedActors)
		{
			for (auto ObjectItem : Item.Value.Selected)
			{
				if(!ObjectItem) { continue; }
				GEditor->SelectActor((AActor*)ObjectItem, true, false);
			}
		}
	});

	Settings->WatchProperty(Settings->PinGeometry, [&](TArray<AStaticMeshActor*>)
	{
		GEditor->GetSelectedActors()->DeselectAll();
		SelectedActors.Add(EDrapeToolStage_SingleClick::Pin, FObjectSelection(Settings->PinGeometry));
		for (auto Item : SelectedActors)
		{
			for (auto ObjectItem : Item.Value.Selected)
			{
				if(!ObjectItem) { continue; }
				GEditor->SelectActor((AActor*)ObjectItem, true, false);
			}
		}
	});
}

void UClothSimTool_Drape::Setup()
{
	Super::Setup();
	
	GEditor->GetSelectedActors()->DeselectAll();
	
	EToolsFrameworkOutcomePins ActionsCreationOutcome;
	Actions = Cast<UClothSimToolActions>(AddPropertySetOfType(UClothSimToolActions::StaticClass(), "Actions", ActionsCreationOutcome));
	if (Actions)
	{
		Actions->Initialize(this);
	}


	EToolsFrameworkOutcomePins SettingsCreationOutcome;
	Settings = Cast<UClothSimPropertySettings>(AddPropertySetOfType(UClothSimPropertySettings::StaticClass(), "Settings", SettingsCreationOutcome));
	
	StartWatchingProperties();

	
	if (GetHandyManAPI())
	{
		CurrentInstance = GetHandyManAPI()->GetMutableHoudiniAPI()->InstantiateAsset
		(
			GetHandyManAPI()->GetHoudiniDigitalAsset(EHandyManToolName::DrapeTool),
			FTransform::Identity,
			nullptr,
			nullptr,
			false
		);
	}
}

void UClothSimTool_Drape::OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	Super::OnHitByClick_Implementation(ClickPos, Modifiers);
}

void UClothSimTool_Drape::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);
}

void UClothSimTool_Drape::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);

	if (!Settings) return;
	
	if (!Settings->bKeepInputClothGeometry)
	{
		Settings->ClothGeometry->Destroy();
	}

	GEditor->GetSelectedActors()->DeselectAll();
}

