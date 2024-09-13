// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManBaseClasses/HandyManModularTool.h"
#include "HandyManSettings.h"

UBaseScriptableToolBuilder* UHandyManModularTool::GetHandyManToolBuilderInstance(UObject* Outer)
{
	return GetNewCustomToolBuilderInstance(Outer);
}

void UHandyManModularTool::Setup()
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
