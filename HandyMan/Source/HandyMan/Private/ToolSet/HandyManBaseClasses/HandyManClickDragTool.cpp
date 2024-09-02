// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManBaseClasses/HandyManClickDragTool.h"

#include "HandyManSettings.h"
#include "ToolSet/Core/HandyManSubsystem.h"

#if WITH_EDITOR
void UHandyManClickDragTool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	/*if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, ToolName) && !ToolName.IsEmpty())
	{
		// Update Our setting

		if (UHandyManSettings* HandyMan = GetMutableDefault<UHandyManSettings>())
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
		}
	}*/
}

void UHandyManClickDragTool::Setup()
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
#endif
