// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTools/ScriptableModularBehaviorTool.h"
#include "Interfaces/HandyManToolInterface.h"
#include "HandyManModularTool.generated.h"

class UHandyManSubsystem;
/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManModularTool : public UScriptableModularBehaviorTool, public IHandyManToolInterface
{
	GENERATED_BODY()

public:

	virtual UBaseScriptableToolBuilder* GetHandyManToolBuilderInstance(UObject* Outer) override;

	virtual bool Trace(FHitResult& OutHit, const FInputDeviceRay& DevicePos) override {return false;};
	virtual bool Trace(TArray<FHitResult>& OutHit, const FInputDeviceRay& DevicePos) override {return false;};


	const UHandyManSubsystem* GetHandyManAPI_Safe() const {return HandyManAPI;}
	UHandyManSubsystem* GetHandyManAPI() const {return HandyManAPI;}

	virtual void Setup() override;


private:
	UPROPERTY()
	TObjectPtr<UHandyManSubsystem> HandyManAPI;
};
