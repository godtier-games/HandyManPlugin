// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTools/ScriptableSingleClickTool.h"
#include "ToolSet/Core/HandyManSubsystem.h"
#include "HandyManSingleClickTool.generated.h"


class UHoudiniAsset;
class UHoudiniPublicAPI;
/**
 * 
 */
UCLASS(Abstract)
class HANDYMAN_API UHandyManSingleClickTool : public UScriptableSingleClickTool
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

#if WITH_EDITORONLY_DATA
	FText LastKnownName;
#endif
	
	const UHandyManSubsystem* GetHandyManAPI_Safe() const {return HandyManAPI;}
	UHandyManSubsystem* GetHandyManAPI() const {return HandyManAPI;}
	

	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);

private:
	UPROPERTY()
	UHandyManSubsystem* HandyManAPI;
};
