#pragma once

#include "HandyManDataTypes.generated.h"

USTRUCT(BlueprintType)
struct FObjectSelection
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<UObject*> Selected;

	FObjectSelection()
	{
		Selected.Empty();
	}

	FObjectSelection(UObject* InObject)
	{
		Selected.Add(InObject);
	};

	FObjectSelection(const TArray<UObject*>& InObjects)
	{
		Selected = InObjects;
	};

	
	
};
