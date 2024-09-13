// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BuildingGeneratorTypes.generated.h"

class UDynamicMesh;

UENUM(BlueprintType)
enum class EMeshBaseShape : uint8
{
	/* Base shape is a cube or rectangle */
	Box,

	/* Base shape is cylindrical or spherical */
	Round,

	/* Combination of Box and Round */
	Combination
};

USTRUCT()
struct HANDYMAN_API FGeneratedOpening
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform = FTransform::Identity;

	UPROPERTY()
	TObjectPtr<AActor> Mesh;

	UPROPERTY()
	bool bShouldCutHoleInTargetMesh = false;

	UPROPERTY()
	EMeshBaseShape BaseShape = EMeshBaseShape::Box;

	bool operator==(const FGeneratedOpening& Other) const
	{
		return Other.Mesh == Mesh && Other.Transform.Equals(Transform);
	}

	bool operator!=(const FGeneratedOpening& Other) const
	{
		return !(Other == *this);
	}


};

USTRUCT(BlueprintType)
struct HANDYMAN_API FDynamicOpening
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UObject> Mesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bShouldCutHoleInTargetMesh = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bShouldCutHoleInTargetMesh", EditConditionHides))
	EMeshBaseShape BaseShape = EMeshBaseShape::Box;
};
