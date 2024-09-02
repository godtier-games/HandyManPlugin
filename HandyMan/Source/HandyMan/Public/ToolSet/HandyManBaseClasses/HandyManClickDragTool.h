// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTools/ScriptableClickDragTool.h"
#include "HandyManClickDragTool.generated.h"

class UHandyManSubsystem;
/**
 * 
 */
UCLASS(Abstract)
class HANDYMAN_API UHandyManClickDragTool : public UScriptableClickDragTool
{
	GENERATED_BODY()

public:

	const UHandyManSubsystem* GetHandyManAPI_Safe() const {return HandyManAPI;}
	UHandyManSubsystem* GetHandyManAPI() const {return HandyManAPI;}
	
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
