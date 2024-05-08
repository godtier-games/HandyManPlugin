// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/Houdini/WorldBuilding/SplineTool/Children/DrawSpline_Equal.h"

#include "HoudiniPublicAPIAssetWrapper.h"
#include "InteractiveToolManager.h"
#include "ToolBuilderUtil.h"

UDrawSpline_Equal::UDrawSpline_Equal()
{
	ToolIdentifier = EHandyManToolName::FenceTool_Easy;
	CurveType = EHoudiniPublicAPICurveType::Bezier;
	DrawMode = EDrawSplineDrawMode_HandyMan::TangentDrag;
}

UBaseScriptableToolBuilder* UDrawSpline_Equal::GetNewCustomToolBuilderInstance(UObject* Outer)
{
	return Cast<UBaseScriptableToolBuilder>(NewObject<UDrawSplineBuilder_Equal>(Outer));
}

UInteractiveTool* UDrawSplineBuilder_Equal::BuildTool(const FToolBuilderState& SceneState) const
{
	UDrawSpline* NewTool = NewObject<UDrawSpline>(SceneState.ToolManager);
	NewTool->SetWorld(SceneState.World);
	NewTool->DrawMode = EDrawSplineDrawMode_HandyMan::TangentDrag;
	NewTool->CurveType = EHoudiniPublicAPICurveType::Bezier;
	NewTool->ToolIdentifier = EHandyManToolName::FenceTool_Easy;

	// May be null
	NewTool->SetSelectedActor(ToolBuilderUtil::FindFirstActor(SceneState, [](AActor*) { return true; }));

	return NewTool;
}


