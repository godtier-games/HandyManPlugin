// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManTools/Houdini/WorldBuilding/SplineTool/DrawSpline.h"
#include "DrawSpline_Equal.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UDrawSpline_Equal : public UDrawSpline
{
	GENERATED_BODY()

public:

	UDrawSpline_Equal();
	virtual UBaseScriptableToolBuilder* GetNewCustomToolBuilderInstance(UObject* Outer) override;
};


UCLASS(Transient)
class HANDYMAN_API UDrawSplineBuilder_Equal : public UHandyManToolBuilder
{
	GENERATED_BODY()

public:
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};

