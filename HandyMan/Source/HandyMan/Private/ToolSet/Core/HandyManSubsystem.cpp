// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/Core/HandyManSubsystem.h"

#include "HandyManSettings.h"
#include "HoudiniPublicAPI.h"
#include "ToolSet/Core/HoudiniAssetWrapper.h"

void UHandyManSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

UHoudiniAsset* UHandyManSubsystem::GetHoudiniDigitalAsset(const EHandyManToolName& ToolName) const
{
	if (const UHandyManSettings* HandyManSettings = GetMutableDefault<UHandyManSettings>())
	{
		if (UHoudiniAssetWrapper* DigitalAssetLibrary = HandyManSettings->GetDigitalAssetLibrary())
		{
			if (DigitalAssetLibrary->DigitalAssets.Contains(ToolName))
			{
				return DigitalAssetLibrary->DigitalAssets.Find(ToolName)->LoadSynchronous();
			}
		}
	}

	return nullptr;
	
}

void UHandyManSubsystem::InitializeHoudiniApi()
{
	if (!HoudiniPublicAPI)
	{
		HoudiniPublicAPI = NewObject<UHoudiniPublicAPI>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
	}

	if (!HoudiniPublicAPI->IsSessionValid())
	{
		HoudiniPublicAPI->CreateSession();
	}
}

void UHandyManSubsystem::CleanUp()
{
	if (!HoudiniPublicAPI)
	{
		HoudiniPublicAPI = NewObject<UHoudiniPublicAPI>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
	}
	
}

