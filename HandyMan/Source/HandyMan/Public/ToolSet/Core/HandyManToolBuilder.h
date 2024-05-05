// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveToolBuilder.h"
#include "ScriptableToolBuilder.h"
#include "HandyManToolBuilder.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManToolBuilder : public UBaseScriptableToolBuilder
{
	GENERATED_BODY()

	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
};
