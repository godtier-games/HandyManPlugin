﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManBaseClasses/HandyManInteractiveTool.h"

#include "HandyManSettings.h"

#if WITH_EDITOR
void UHandyManInteractiveTool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
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


UBaseScriptableToolBuilder* UHandyManInteractiveTool::GetHandyManToolBuilderInstance(UObject* Outer)
{
	return GetNewCustomToolBuilderInstance(Outer);
}

void UHandyManInteractiveTool::Setup()
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
