// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PCGToolInterface.generated.h"

class UPCGComponent;
// This class does not need to be modified.
UINTERFACE(NotBlueprintable)
class UPCGToolInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HANDYMAN_API IPCGToolInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintCallable, Category = "PCG")
	virtual UPCGComponent* GetPCGComponent() const = 0;
};
