﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScriptableInteractiveTool.h"
#include "Interfaces/HandyManToolInterface.h"
#include "ToolSet/Core/HandyManSubsystem.h"
#include "HandyManInteractiveTool.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManInteractiveTool : public UScriptableInteractiveTool, public IHandyManToolInterface
{
	GENERATED_BODY()

public:
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

#if WITH_EDITORONLY_DATA
	FText LastKnownName;
#endif
	

	const UHandyManSubsystem* GetHandyManAPI_Safe() const {return HandyManAPI;}
	UHandyManSubsystem* GetHandyManAPI() const {return HandyManAPI;}

	virtual UBaseScriptableToolBuilder* GetHandyManToolBuilderInstance(UObject* Outer) override;
	virtual bool Trace(FHitResult& OutHit, const FInputDeviceRay& DevicePos) override {return false;};
	friend class UHandyManScriptableToolSet;


	virtual void Setup() override;


private:
	UPROPERTY()
	TObjectPtr<UHandyManSubsystem> HandyManAPI;
};
