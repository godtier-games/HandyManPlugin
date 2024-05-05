// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScriptableInteractiveTool.h"
#include "HandyManInteractiveTool.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManInteractiveTool : public UScriptableInteractiveTool
{
	GENERATED_BODY()

public:

	virtual UBaseScriptableToolBuilder* GetNewCustomToolBuilderInstance(UObject* Outer) override {return nullptr;};
};
