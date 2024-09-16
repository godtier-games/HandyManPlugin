// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModelingUtilities/ModelingUtilitiesDataTypes.h"
#include "UObject/Object.h"
#include "BuildingGeneratorTypes.generated.h"

class UDynamicMesh;


USTRUCT()
struct HANDYMAN_API FGeneratedOpening
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform = FTransform::Identity;

	UPROPERTY()
	TObjectPtr<AActor> Mesh;
	
	UPROPERTY()
	bool bShouldLayFlush = false;

	UPROPERTY()
	bool bShouldSnapToGroundSurface = false;

	UPROPERTY()
	bool bShouldApplyBoolean = false;

	UPROPERTY()
	bool bShouldCutHoleInTargetMesh = false;

	UPROPERTY()
	EMeshBooleanShape BooleanShape = EMeshBooleanShape::Box;

	bool operator==(const FGeneratedOpening& Other) const
	{
		return Other.Mesh == Mesh && Other.Transform.Equals(Transform);
	}

	bool operator!=(const FGeneratedOpening& Other) const
	{
		return !(Other == *this);
	}
};

USTRUCT()
struct HANDYMAN_API FGeneratedOpeningArray
{
	GENERATED_BODY()

	FGeneratedOpeningArray() = default;

	FGeneratedOpeningArray(const TArray<FGeneratedOpening>& InOpenings)
	{
		Openings = InOpenings;
	}

	FGeneratedOpeningArray(const FGeneratedOpening& Entry)
	{
		Openings.Add(Entry);
	}

	UPROPERTY()
	TArray<FGeneratedOpening> Openings;

	bool Contains(const FGeneratedOpening& Other) const
	{
		for (int i = 0; i < Openings.Num(); ++i)
		{
			if(Openings[i].Mesh && Other.Mesh && Openings[i].Mesh.GetFName() == Other.Mesh.GetFName())
			{
				return true;
			}
		}

		return false;
	}
};




USTRUCT(BlueprintType)
struct HANDYMAN_API FDynamicOpening
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UObject> Mesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bShouldLayFlush = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bShouldSnapToGroundSurface = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bShouldApplyBoolean = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bShouldApplyBoolean", EditConditionHides))
	bool bIsSubtractiveBoolean = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bShouldApplyBoolean", EditConditionHides))
	EMeshBooleanShape BooleanShape = EMeshBooleanShape::Exact;
};

USTRUCT()
struct HANDYMAN_API FGeneratedClutter
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform = FTransform::Identity;

	UPROPERTY()
	TObjectPtr<AActor> Mesh;

	bool operator==(const FGeneratedClutter& Other) const
	{
		return Other.Mesh == Mesh && Other.Transform.Equals(Transform);
	}

	bool operator!=(const FGeneratedClutter& Other) const
	{
		return !(Other == *this);
	}
};

USTRUCT(BlueprintType)
struct HANDYMAN_API FDynamicClutter
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UObject> Mesh;
};

