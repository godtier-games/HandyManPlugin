// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Tags/HandyManToolGroupSet.h"

UHandyManToolGroupSet::UHandyManToolGroupSet()
	: GroupsProperty(nullptr)
{
	if (!IsTemplate())
	{
		GroupsProperty = FindFProperty<FProperty>(GetClass(), TEXT("GroupSet"));
	}
}

void UHandyManToolGroupSet::SetGroups(const FScriptableToolGroupSet::FGroupSet& InGroups)
{
	GroupSet.SetGroups(InGroups);
}

FScriptableToolGroupSet::FGroupSet& UHandyManToolGroupSet::GetGroups()
{
	return GroupSet.GetGroups();
}

FString UHandyManToolGroupSet::GetGroupSetExportText()
{
	GroupsPropertyAsString.Reset();

	if (GroupsProperty)
	{
		GroupsProperty->ExportTextItem_Direct(GroupsPropertyAsString, &GroupSet, &GroupSet, this, 0);
	}

	return GroupsPropertyAsString;
}