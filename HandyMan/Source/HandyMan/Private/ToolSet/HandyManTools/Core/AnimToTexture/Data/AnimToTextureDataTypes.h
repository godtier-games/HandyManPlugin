// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimToTextureDataTypes.generated.h"

class UAnimToTextureDataAsset;

UENUM()
enum class EAnimTextureResolution : uint8
{
	Size_64 UMETA(DisplayName = "64px"),
	Size_128 UMETA(DisplayName = "128px"),
	Size_256 UMETA(DisplayName = "256px"),
	Size_512 UMETA(DisplayName = "512px"),
	Size_1024 UMETA(DisplayName = "1k"),
	Size_2048 UMETA(DisplayName = "2k"),
	Size_4096 UMETA(DisplayName = "4k"),
	Size_8192 UMETA(DisplayName = "8k"),
};
/**
 * 
 */
USTRUCT(BlueprintType)
struct FAnimToTextureData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> Source;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAnimTextureResolution Resolution = EAnimTextureResolution::Size_8192;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UAnimSequence>> AnimationsToBake;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMesh> Target;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimToTextureDataAsset> Data;
	
};