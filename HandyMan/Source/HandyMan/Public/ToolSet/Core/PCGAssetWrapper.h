// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_ActorBase.h"
#include "PCGAssetWrapper.generated.h"

enum class EHandyManToolName;

/**
 * 
 */
UCLASS()
class HANDYMAN_API UPCGAssetWrapper : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PCG", meta=(GetOptions="HandyMan.HandyManStatics.GetToolNames"))
	TMap<FName, TSubclassOf<AActor>> ActorClasses;
};
