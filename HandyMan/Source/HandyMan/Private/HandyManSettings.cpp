// Fill out your copyright notice in the Description page of Project Settings.


#include "HandyManSettings.h"

#include "ToolSet/Core/PCGAssetWrapper.h"


#define LOCTEXT_NAMESPACE "HandyManModeSettings"


UHandyManSettings::UHandyManSettings()
{
}

FText UHandyManSettings::GetSectionText() const 
{ 
	return LOCTEXT("HandyManSettingsName", "Handy Man"); 
}

FText UHandyManSettings::GetSectionDescription() const
{
	return LOCTEXT("HandyManSettingsDescription", "Configure the Handy Man Editor Mode plugin");
}

/*UHoudiniAssetWrapper* UHandyManSettings::GetDigitalAssetLibrary() const
{
	FString AssetPath = DigitalAssetLibrary.ToString();
	return DigitalAssetLibrary.IsValid() ? Cast<UHoudiniAssetWrapper>(DigitalAssetLibrary.TryLoad()) : nullptr;
}*/

UPCGAssetWrapper* UHandyManSettings::GetPCGActorLibrary() const
{
	FString AssetPath = PCGActorLibrary.ToString();
	return PCGActorLibrary.IsValid() ? Cast<UPCGAssetWrapper>(PCGActorLibrary.TryLoad()) : nullptr;
}

void UHandyManSettings::RemoveToolName(const FName& ToolName)
{
	for (int i = ToolNames.Num() - 1; i >= 0; --i)
	{
		if (ToolNames[i].IsEqual(ToolName))
		{
			ToolNames.RemoveAtSwap(i);
			break;
		}
	}
}

void UHandyManSettings::AddToolName(const FName& ToolName)
{
	bool bShouldAdd = true;
	for (int i = 0; i < ToolNames.Num(); ++i)
    {
        if (ToolNames[i].IsEqual(ToolName))
        {
            bShouldAdd = false;
            break;
        }
    }

	if (bShouldAdd)
	{
		ToolNames.Add(ToolName);
	}
}

UDataTable* UHandyManSettings::GetBuildingModuleDataTable()
{
	FString AssetPath = BuildingModuleDataTable.ToString();
	return BuildingModuleDataTable.IsValid() ? Cast<UDataTable>(BuildingModuleDataTable.TryLoad()) : nullptr;
}

UDataTable* UHandyManSettings::GetBuildingMeshesDataTable()
{
	FString AssetPath = BuildingMeshesDataTable.ToString();
	return BuildingMeshesDataTable.IsValid() ? Cast<UDataTable>(BuildingMeshesDataTable.TryLoad()) : nullptr;
}


FText UHandyManCustomizationSettings::GetSectionText() const 
{ 
	return LOCTEXT("HandyManSettingsName", "Handy Man"); 
}

FText UHandyManCustomizationSettings::GetSectionDescription() const
{
	return LOCTEXT("HandyManSettingsDescription", "Configure the Handy Man Editor Mode plugin");
}


#undef LOCTEXT_NAMESPACE