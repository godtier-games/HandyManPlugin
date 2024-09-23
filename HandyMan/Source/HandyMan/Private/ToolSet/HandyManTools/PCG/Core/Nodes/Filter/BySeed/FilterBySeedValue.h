// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "UObject/Object.h"
#include "FilterBySeedValue.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, ClassGroup = (HandyMan))
class HANDYMAN_API UFilterBySeedValueSettings : public UPCGSettings
{
	GENERATED_BODY()
public:
	UFilterBySeedValueSettings();

	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("FilterBySeedValue")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGFilterBySeedValueSettings", "NodeTitle", "Filter By Seed Value"); }
	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGFilterBySeedValueSettings", "NodeTooltip", "Compare the point array to the input value and return it if it matches."); }
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
	/** The seed value we want the matching point to have */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	int32 SeedToCompare = 0;
};

class FPCGFilterBySeedValueElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;


	// Cache the seed value of the selected points so there are no duplicates
};

