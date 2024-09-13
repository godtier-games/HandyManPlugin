// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BuildingGeneratorTypes.generated.h"

class UDynamicMesh;

USTRUCT()
struct HANDYMAN_API FGeneratedOpening
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	TObjectPtr<AActor> Mesh;

	UPROPERTY()
	TObjectPtr<UDynamicMesh> BooleanMesh;
};

USTRUCT(BlueprintType)
struct HANDYMAN_API FDynamicOpening
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UObject> Mesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bShouldCutHoleInTargetMesh = false;
};
