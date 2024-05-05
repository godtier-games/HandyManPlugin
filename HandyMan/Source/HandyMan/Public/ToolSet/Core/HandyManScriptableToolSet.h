// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScriptableToolSet.h"
#include "HandyManScriptableToolSet.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManScriptableToolSet : public UScriptableToolSet
{
	GENERATED_BODY()

public:
	void ReinitializeCustomScriptableTools();

	
};
