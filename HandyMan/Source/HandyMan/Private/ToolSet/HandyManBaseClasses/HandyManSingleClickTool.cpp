// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"


#include "HandyManSettings.h"
#include "Interfaces/IPluginManager.h"

#define HDA( RelativePath, ... ) FString( UHandyManSingleClickTool::InContent(RelativePath, ".uasset" ))

// This is to fix the issue that SlateStyleMacros like IMAGE_BRUSH look for RootToContentDir but StyleSet->RootToContentDir is how this style is set up
#define RootToContentDir StyleSet->RootToContentDir

#if WITH_EDITOR
void UHandyManSingleClickTool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, ToolName) && !ToolName.IsEmpty())
	{
		// Update Our setting

		/*if (UHandyManSettings* HandyMan = GetMutableDefault<UHandyManSettings>())
		{
			if (!LastKnownName.IsEmpty())
			{
				// Remove this from the array
				HandyMan->RemoveToolName(FName(LastKnownName.ToString()));
			}

			// Add new name to the Array
			HandyMan->AddToolName(FName(ToolName.ToString()));

			// Update LastKnownName
			LastKnownName = ToolName;
		}*/
	}
}
#endif

void UHandyManSingleClickTool::Setup()
{
	if (UHandyManSettings* HandyMan = GetMutableDefault<UHandyManSettings>())
	{
		if (!HandyMan->GetToolsNames().Contains(FName(ToolName.ToString())))
		{
			HandyMan->AddToolName(FName(ToolName.ToString()));
		}
	}
	
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
