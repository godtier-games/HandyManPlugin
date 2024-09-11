// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/Core/HandyManToolBuilder.h"
#include "ToolSet/HandyManBaseClasses/HandyManInteractiveTool.h"
#include "ToolSet/HandyManTools/PCG/SplineTool/Basic/Tool/HandyManSplineTool.h"
#include "ToolSet/Utils/SplineUtils.h"
#include "HandyManSplineTool_Dynamic.generated.h"

class UDynamicSplineToolProperties;
class APCG_DynamicSplineActor;
class USplineComponent;
class USingleClickOrDragInputBehavior;
class UConstructionPlaneMechanic;
class APreviewGeometryActor;

namespace ESplinePointType
{
	enum Type : int;
}

/**
 * 
 */
UCLASS()
class HANDYMAN_API USplineTool_Dynamic : public USplineTool
{
	GENERATED_BODY()
public:

	USplineTool_Dynamic();

	virtual UBaseScriptableToolBuilder* GetNewCustomToolBuilderInstance(UObject* Outer) override;

	void SpawnActorInstance(const UDynamicSplineToolProperties* CurrentSettings);
	virtual void SetSelectedActor(AActor* Actor) override;

	virtual UWorld* GetTargetWorld() override { return TargetWorld.Get();}
	
};



#pragma region BUILDER

UCLASS(Transient)
class HANDYMAN_API UDynamicSplineToolBuilder : public UHandyManToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;

	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};

#pragma endregion


#pragma region SETTINGS


/**
 * 
 */
#pragma endregion
