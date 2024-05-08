// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/Houdini/WorldBuilding/SplineTool/Children/DrawSpline_Smooth.h"

#include "HoudiniPublicAPIAssetWrapper.h"
#include "InteractiveToolManager.h"
#include "ToolBuilderUtil.h"

UDrawSpline_Smooth::UDrawSpline_Smooth()
{
	
}

UBaseScriptableToolBuilder* UDrawSpline_Smooth::GetNewCustomToolBuilderInstance(UObject* Outer)
{
	return Cast<UBaseScriptableToolBuilder>(NewObject<UDrawSplineBuilder_Smooth>(Outer));

	
}

UInteractiveTool* UDrawSplineBuilder_Smooth::BuildTool(const FToolBuilderState& SceneState) const
{
	UDrawSpline* NewTool = NewObject<UDrawSpline>(SceneState.ToolManager);
	NewTool->SetWorld(SceneState.World);
	NewTool->DrawMode = EDrawSplineDrawMode_HandyMan::None;
	NewTool->CurveType = EHoudiniPublicAPICurveType::Polygon;
	NewTool->ToolIdentifier = EHandyManToolName::FenceTool_Smooth;

	// May be null
	NewTool->SetSelectedActor(ToolBuilderUtil::FindFirstActor(SceneState, [](AActor*) { return true; }));

	return NewTool;
}

