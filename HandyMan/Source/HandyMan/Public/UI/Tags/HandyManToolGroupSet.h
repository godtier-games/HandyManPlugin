// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Tags/ScriptableToolGroupSet.h"
#include "HandyManToolGroupSet.generated.h"

UCLASS(Transient, MinimalAPI)
class UHandyManToolGroupSet : public UObject
{
public:
	GENERATED_BODY()

	UHandyManToolGroupSet();

	void SetGroups(const FScriptableToolGroupSet::FGroupSet& InGroups);
	FScriptableToolGroupSet::FGroupSet& GetGroups();

	FString GetGroupSetExportText();

private:
	UPROPERTY()
	FScriptableToolGroupSet GroupSet;

	FProperty* GroupsProperty;
	FString GroupsPropertyAsString;
};