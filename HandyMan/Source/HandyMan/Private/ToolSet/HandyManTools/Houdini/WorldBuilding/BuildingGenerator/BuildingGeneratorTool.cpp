// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/Houdini/WorldBuilding/BuildingGenerator/BuildingGeneratorTool.h"

#include "InteractiveToolManager.h"

UInteractiveTool* UBuildingGeneratorToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	auto Tool =  NewObject<UBuildingGeneratorTool>(SceneState.ToolManager);
	return Tool;
}

UBaseScriptableToolBuilder* UBuildingGeneratorTool::GetNewCustomToolBuilderInstance(UObject* Outer)
{
	auto Builder =  NewObject<UBuildingGeneratorToolBuilder>(Outer);
	return Builder;
}
