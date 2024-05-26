// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/Core/HandyManSubsystem.h"

#include "HandyManSettings.h"
#include "HoudiniPublicAPI.h"
#include "ToolSet/Core/HoudiniAssetWrapper.h"
#include "ToolSet/Core/PCGAssetWrapper.h"

void UHandyManSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

UHandyManSettings* UHandyManSubsystem::GetHandyManSettings() const
{
	return GetMutableDefault<UHandyManSettings>();
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

TSubclassOf<AActor> UHandyManSubsystem::GetPCGActorClass(const EHandyManToolName& ToolName) const
{
	if (const UHandyManSettings* HandyManSettings = GetMutableDefault<UHandyManSettings>())
	{
		if (UPCGAssetWrapper* Library = HandyManSettings->GetPCGActorLibrary())
		{
			if (Library->ActorClasses.Contains(ToolName))
			{
				return *Library->ActorClasses.Find(ToolName);
			}
		}
	}

	return nullptr;
}

void UHandyManSubsystem::InitializeHoudiniApi()
{
	// TODO : Enable this when I need houdini engine
	/*if (!HoudiniPublicAPI)
	{
		HoudiniPublicAPI = NewObject<UHoudiniPublicAPI>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
	}

	if (!HoudiniPublicAPI->IsSessionValid())
	{
		HoudiniPublicAPI->CreateSession();
	}*/
}

void UHandyManSubsystem::CleanUp()
{
	if (!HoudiniPublicAPI)
	{
		HoudiniPublicAPI = NewObject<UHoudiniPublicAPI>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
	}
	
}

