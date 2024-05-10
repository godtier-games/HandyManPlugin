// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/Core/HandyManToolBuilder.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "BuildingGeneratorTool.generated.h"


UCLASS()
class HANDYMAN_API UBuildingGeneratorToolBuilder : public UHandyManToolBuilder
{
	GENERATED_BODY()

public:
	//virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};




/**
 * 
 */
UCLASS()
class HANDYMAN_API UBuildingGeneratorTool : public UHandyManSingleClickTool
{
	GENERATED_BODY()

public:
	virtual UBaseScriptableToolBuilder* GetNewCustomToolBuilderInstance(UObject* Outer) override;
};
