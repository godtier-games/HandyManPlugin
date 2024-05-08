// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManTools/Houdini/WorldBuilding/SplineTool/DrawSpline.h"
#include "DrawSpline_Smooth.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UDrawSpline_Smooth : public UDrawSpline
{
	GENERATED_BODY()

public:
	UDrawSpline_Smooth();

	virtual UBaseScriptableToolBuilder* GetNewCustomToolBuilderInstance(UObject* Outer) override;
};

UCLASS(Transient)
class HANDYMAN_API UDrawSplineBuilder_Smooth : public UHandyManToolBuilder
{
	GENERATED_BODY()

public:


	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};

