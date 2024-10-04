// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScriptableInteractiveTool.h"
#include "HandyManToolPropertySet.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManToolPropertySet : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	
	// Access the property sets parent tool
	UScriptableInteractiveTool* GetParentTool() const {return ParentTool.Get();}

	// Typed accessor to this property sets parent tool
	template<class T>
	T* GetParentTool() const
	{
		return Cast<T>(GetParentTool());
	}
	
};
