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

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<USkeletalMesh> InputMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString SectionName;
};

UENUM()
enum class ECutterShapeType : uint8
{
	Box,
	Sphere,
};
