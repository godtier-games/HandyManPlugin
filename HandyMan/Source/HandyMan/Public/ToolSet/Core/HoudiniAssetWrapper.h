// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HoudiniAsset.h"
#include "Engine/DataAsset.h"
#include "HoudiniAssetWrapper.generated.h"


enum class EHandyManToolName;
/**
 * 
 */
UCLASS()
class HANDYMAN_API UHoudiniAssetWrapper : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Houdini")
	TMap<EHandyManToolName,TSoftObjectPtr<UHoudiniAsset> > DigitalAssets;
};
