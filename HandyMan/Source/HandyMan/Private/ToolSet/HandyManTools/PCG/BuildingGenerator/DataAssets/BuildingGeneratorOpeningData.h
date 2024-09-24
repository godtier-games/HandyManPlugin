// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ToolSet/HandyManTools/PCG/BuildingGenerator/DataTypes/BuildingGeneratorTypes.h"
#include "BuildingGeneratorOpeningData.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UBuildingGeneratorOpeningData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FDynamicOpening> Openings;
};
