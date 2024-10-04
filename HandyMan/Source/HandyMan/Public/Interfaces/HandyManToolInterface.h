// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputState.h"
#include "UObject/Interface.h"
#include "HandyManToolInterface.generated.h"

class UBaseScriptableToolBuilder;
// This class does not need to be modified.
UINTERFACE()
class UHandyManToolInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HANDYMAN_API IHandyManToolInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// return instance of custom tool builder. Should only be called on CDO.
	virtual UBaseScriptableToolBuilder* GetHandyManToolBuilderInstance(UObject* Outer) = 0;

	virtual bool Trace(FHitResult& OutHit, const FInputDeviceRay& DevicePos) {return false;};

	virtual bool Trace(TArray<FHitResult>& OutHit, const FInputDeviceRay& DevicePos) {return false;};
};
