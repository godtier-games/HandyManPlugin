// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ToolSet/DataTypes/HandyManDataTypes.h"
#include "HM_ScatterToolGeometryData.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHM_ScatterToolGeometryData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mesh Data")
	TArray<FHandyManDynamicMeshData> Meshes;
};
