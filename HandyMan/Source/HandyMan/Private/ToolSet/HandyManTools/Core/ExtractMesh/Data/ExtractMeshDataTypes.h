// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExtractMeshDataTypes.generated.h"

class UDynamicMesh;

USTRUCT(BlueprintType)
struct HANDYMAN_API FExtractedMeshInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName CustomMeshName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bVisible = true;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<UMaterialInterface> MaterialID;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<TObjectPtr<UDynamicMesh>> MeshLods;
	
};