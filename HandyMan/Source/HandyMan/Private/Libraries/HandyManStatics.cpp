// Fill out your copyright notice in the Description page of Project Settings.


#include "Libraries/HandyManStatics.h"

#include "GeneralEngineSettings.h"
#include "HandyManSettings.h"
#include "Editor/UnrealEdEngine.h"

TArray<FName> UHandyManStatics::GetToolNames()
{
	TArray<FName> ReturnValue;

	if (auto Settings = GetMutableDefault<UHandyManSettings>())
	{
		for (const auto& Item  : Settings->GetToolsNames())
		{
			ReturnValue.Add(Item);
		}
	}
	
	return ReturnValue;
}

#if WITH_EDITOR
TArray<FName> UHandyManStatics::GetCollisionProfileNames()
{
	TArray<TSharedPtr<FName>> SharedNames;
	UCollisionProfile::GetProfileNames(SharedNames);

	TArray<FName> Names;
	Names.Reserve(SharedNames.Num());
	for (const TSharedPtr<FName>& SharedName : SharedNames)
	{
		if (const FName* Name = SharedName.Get())
		{
			Names.Add(*Name);
		}
	}

	return Names;
}

#endif

