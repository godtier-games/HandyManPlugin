// Fill out your copyright notice in the Description page of Project Settings.


#include "HandyManSettings.h"

#include "ToolSet/Core/HoudiniAssetWrapper.h"


#define LOCTEXT_NAMESPACE "HandyManModeSettings"


FText UHandyManSettings::GetSectionText() const 
{ 
	return LOCTEXT("HandyManSettingsName", "Handy Man"); 
}

FText UHandyManSettings::GetSectionDescription() const
{
	return LOCTEXT("HandyManSettingsDescription", "Configure the Handy Man Editor Mode plugin");
}

UHoudiniAssetWrapper* UHandyManSettings::GetDigitalAssetLibrary() const
{
	FString AssetPath = DigitalAssetLibrary.ToString();
	return DigitalAssetLibrary.IsValid() ? Cast<UHoudiniAssetWrapper>(DigitalAssetLibrary.TryLoad()) : nullptr;
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