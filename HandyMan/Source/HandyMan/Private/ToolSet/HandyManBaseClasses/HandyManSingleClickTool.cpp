// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"

#include "HoudiniAsset.h"
#include "HoudiniPublicAPI.h"
#include "Interfaces/IPluginManager.h"
#include "Subsystems/EditorAssetSubsystem.h"

#define HDA( RelativePath, ... ) FString( UHandyManSingleClickTool::InContent(RelativePath, ".uasset" ))

// This is to fix the issue that SlateStyleMacros like IMAGE_BRUSH look for RootToContentDir but StyleSet->RootToContentDir is how this style is set up
#define RootToContentDir StyleSet->RootToContentDir

void UHandyManSingleClickTool::CacheHoudiniAPI()
{
	if (!HoudiniPublicAPI)
	{
		HoudiniPublicAPI = NewObject<UHoudiniPublicAPI>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
	}
}

void UHandyManSingleClickTool::InitHoudiniDigitalAsset()
{
	if (HDAFileName.IsEmpty() == false)
	{
		if (UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>())
		{
			FString FilePath = HDA(HDAFileName);
			auto Asset = EditorAssetSubsystem->LoadAsset(FilePath);
			if (!Asset)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to load HDA"));
				return;
			}

			HDA = Cast<UHoudiniAsset>(Asset);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HDA File Path is empty"));
	}
		


}

void UHandyManSingleClickTool::Setup()
{
	Super::Setup();
	CacheHoudiniAPI();
	//InitHoudiniDigitalAsset();
}

void UHandyManSingleClickTool::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);

	HoudiniPublicAPI = nullptr;
}


// This is to fix the issue that SlateStyleMacros like IMAGE_BRUSH look for RootToContentDir but StyleSet->RootToContentDir is how this style is set up
#define RootToContentDir StyleSet->RootToContentDir

FString UHandyManSingleClickTool::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("HandyMan"))->GetContentDir();
	return (ContentDir / "HDA/" / RelativePath) + Extension;
}
