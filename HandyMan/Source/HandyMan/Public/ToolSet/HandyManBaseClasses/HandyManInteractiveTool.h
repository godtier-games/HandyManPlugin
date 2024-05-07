// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScriptableInteractiveTool.h"
#include "ToolSet/Core/HandyManSubsystem.h"
#include "HandyManInteractiveTool.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManInteractiveTool : public UScriptableInteractiveTool
{
	GENERATED_BODY()

public:

	const UHandyManSubsystem* GetHandyManAPI_Safe() const {return HandyManAPI;}
	UHandyManSubsystem* GetHandyManAPI() const {return HandyManAPI;}

	virtual UBaseScriptableToolBuilder* GetNewCustomToolBuilderInstance(UObject* Outer) override {return nullptr;};
	friend class UHandyManScriptableToolSet;


	virtual void Setup() override;


private:
	UPROPERTY()
	UHandyManSubsystem* HandyManAPI;
};
