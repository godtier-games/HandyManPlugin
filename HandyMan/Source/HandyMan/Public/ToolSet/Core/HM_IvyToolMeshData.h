// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ToolSet/DataTypes/HandyManDataTypes.h"
#include "HM_IvyToolMeshData.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHM_IvyToolMeshData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ivy Tool Mesh Data")
	TSoftObjectPtr<UMaterialInterface> VineMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ivy Tool Mesh Data")
	TArray<FHandyManDynamicMeshData> LeafMeshes;
};
