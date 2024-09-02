// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HandyManStatics.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintPure)
	static TArray<FName> GetToolNames();

#if WITH_EDITOR
	UFUNCTION(BlueprintPure)
	static TArray<FName> GetCollisionProfileNames();
#endif
	
};
