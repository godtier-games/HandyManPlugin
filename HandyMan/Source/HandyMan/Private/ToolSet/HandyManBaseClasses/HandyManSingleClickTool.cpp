// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"

#include "HoudiniAsset.h"
#include "HoudiniPublicAPI.h"
#include "Interfaces/IPluginManager.h"
#include "Subsystems/EditorAssetSubsystem.h"

#define HDA( RelativePath, ... ) FString( UHandyManSingleClickTool::InContent(RelativePath, ".uasset" ))

// This is to fix the issue that SlateStyleMacros like IMAGE_BRUSH look for RootToContentDir but StyleSet->RootToContentDir is how this style is set up
#define RootToContentDir StyleSet->RootToContentDir

void UHandyManSingleClickTool::Setup()
{
	Super::Setup();
	HandyManAPI = GEditor->GetEditorSubsystem<UHandyManSubsystem>();
}

void UHandyManSingleClickTool::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);
}


// This is to fix the issue that SlateStyleMacros like IMAGE_BRUSH look for RootToContentDir but StyleSet->RootToContentDir is how this style is set up
#define RootToContentDir StyleSet->RootToContentDir

FString UHandyManSingleClickTool::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("HandyMan"))->GetContentDir();
	return (ContentDir / "HDA/" / RelativePath) + Extension;
}
