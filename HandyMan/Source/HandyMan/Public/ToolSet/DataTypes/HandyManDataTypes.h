#pragma once
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"

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
		Selected.Empty();
		Selected.Add(InObject);
	};

	FObjectSelection(const TArray<UObject*>& InObjects)
	{
		Selected.Empty();
		Selected = InObjects;
	};

	FObjectSelection(const TArray<AActor*>& InObjects)
	{
		Selected.Empty();
		for (auto Item : InObjects)
		{
			Selected.Add(Cast<UObject>(Item));
		}
	};

	FObjectSelection(const TArray<AStaticMeshActor*>& InObjects)
	{
		Selected.Empty();
		for (auto Item : InObjects)
		{
			Selected.Add(Cast<UObject>(Item));
		}
	};

	
	
};
