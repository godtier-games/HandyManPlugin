// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SkeletalMeshCutterTypes.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FSkeletalMeshAssetData
{
	GENERATED_BODY()

	FSkeletalMeshAssetData()
	{
		LODSToWrite.Add(0);
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<USkeletalMesh> InputMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bApplyChangesToAllLODS = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition = "!bApplyChangesToAllLODS", EditConditionHides))
	TArray<int32> LODSToWrite;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, NoClear)
	FString FolderName = TEXT("GENERATED");

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString SectionName;

	bool IsValid() const
	{
		const bool ShouldReturn = InputMesh.Get() != nullptr && !FolderName.IsEmpty() && !SectionName.IsEmpty();

		if (!bApplyChangesToAllLODS)
		{
			return ShouldReturn && LODSToWrite.Num() > 0;
		}
		
		return ShouldReturn;
	}
};

UENUM()
enum class ECutterShapeType : uint8
{
	Box,
	Sphere,
};
