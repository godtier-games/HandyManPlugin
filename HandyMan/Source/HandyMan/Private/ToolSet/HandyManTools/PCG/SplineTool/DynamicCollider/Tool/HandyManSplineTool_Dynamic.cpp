// Fill out your copyright notice in the Description page of Project Settings.


#include "HandyManSplineTool_Dynamic.h"

#include "AssetSelection.h"
#include "HandyManSettings.h"
#include "InteractiveToolManager.h"
#include "PCGComponent.h"
#include "SplineUtil.h"
#include "ToolBuilderUtil.h"
#include "UnrealEdGlobals.h"
#include "ActorFactories/ActorFactoryEmptyActor.h"
#include "BaseBehaviors/SingleClickOrDragBehavior.h"
#include "BaseGizmos/GizmoMath.h"
#include "Drawing/PreviewGeometryActor.h"
#include "Editor/UnrealEdEngine.h"
#include "Mechanics/ConstructionPlaneMechanic.h"
#include "Selection/ToolSelectionUtil.h"
#include "ToolSet/HandyManTools/PCG/SplineTool/DynamicCollider/Actor/PCG_DynamicSplineActor.h"
#define LOCTEXT_NAMESPACE "USplineTool_Dynamic"


USplineTool_Dynamic::USplineTool_Dynamic()
{
	ToolName = LOCTEXT("SplineToolName", "Rail Tool");
}

UBaseScriptableToolBuilder* USplineTool_Dynamic::GetNewCustomToolBuilderInstance(UObject* Outer)
{
	return Cast<UBaseScriptableToolBuilder>(NewObject<UDynamicSplineToolBuilder>(Outer));
}

using namespace UE::Geometry;

// To be called by builder
void USplineTool_Dynamic::SetSelectedActor(AActor* Actor)
{
	SelectedActor = Actor;
}

/// Tool builder:

bool UDynamicSplineToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return true;
}

UInteractiveTool* UDynamicSplineToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	USplineTool_Dynamic* NewTool = NewObject<USplineTool_Dynamic>(SceneState.ToolManager);
	NewTool->SetTargetWorld(SceneState.World);
	

	// May be null
	NewTool->SetSelectedActor(ToolBuilderUtil::FindFirstActor(SceneState, [](AActor*) { return true; }));
	NewTool->DrawMode = EDrawSplineDrawMode_HandyMan::TangentDrag;

	return NewTool;
}

#undef LOCTEXT_NAMESPACE

