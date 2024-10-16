﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "UObject/Object.h"
#include "GroupRandomSelection.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, ClassGroup = (HandyMan))
class HANDYMAN_API UGroupRandomSelection : public UPCGSettings
{
	GENERATED_BODY()

public:
	UGroupRandomSelection();

	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("GroupRandomSelection")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGGroupRandomSelectionSettings", "NodeTitle", "Group Random Selection"); }
	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGGroupRandomSelectionSettings", "NodeTooltip", "Split a point input in 2, randomly according to a ratio."); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Filter; }
#endif

	virtual FName AdditionalTaskName() const override;

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return Super::DefaultPointInputPinProperties(); }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;
	virtual bool UseSeed() const override {return true;}
	//~End UPCGSettings interface

public:
	/** If True, will use the fixed number of points. Ratio otherwise. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	bool bFixedMode = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, EditCondition = "bFixedMode", EditConditionHides))
	bool bUseMaxFixedValue = false;

	/** Ratio of points to keep. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, ClampMin = 0.0, ClampMax = 1.0, EditCondition = "!bFixedMode", EditConditionHides))
	float Ratio = 0.1f;

	/** Fixed number of points to keep. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, ClampMin = 1, EditCondition = "bFixedMode", EditConditionHides))
	int32 FixedNumber = 1;
};

class FPCGGroupRandomSelectionElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};
