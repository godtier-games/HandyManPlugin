// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScriptableToolBuilder.h"
#include "HandyManToolWithTargetBuilder.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManToolWithTargetBuilder : public UCustomScriptableToolBuilder
{
	GENERATED_BODY()

public:

	UHandyManToolWithTargetBuilder(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandyMan")
	TArray<UClass*> AcceptedClasses;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandyMan")
	int32 MinRequiredMatches = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandyMan")
	int32 MatchLimit = 1;

	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual void SetupTool(const FToolBuilderState& SceneState, UInteractiveTool* Tool) const override;


	FToolTargetTypeRequirements GetRequirements() const;

};
