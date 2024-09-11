// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTools/ScriptableClickDragTool.h"
#include "Interfaces/HandyManPhysicsInterface.h"
#include "Interfaces/HandyManToolInterface.h"
#include "HandyManClickDragTool.generated.h"

class UHandyManSubsystem;
/**
 * 
 */
UCLASS(Abstract)
class HANDYMAN_API UHandyManClickDragTool : public UScriptableClickDragTool, public IHandyManPhysicsInterface, public IHandyManToolInterface
{
	GENERATED_BODY()

public:

	const UHandyManSubsystem* GetHandyManAPI_Safe() const {return HandyManAPI;}
	UHandyManSubsystem* GetHandyManAPI() const {return HandyManAPI;}

	/** Traces the actor under cursor */
	virtual bool Trace(FHitResult& OutHit, const FInputDeviceRay& DevicePos);

	virtual UBaseScriptableToolBuilder* GetHandyManToolBuilderInstance(UObject* Outer) override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

#if WITH_EDITORONLY_DATA
	FText LastKnownName;

	virtual void Setup() override;
#endif


	
private:
	UPROPERTY()
	UHandyManSubsystem* HandyManAPI;
};

