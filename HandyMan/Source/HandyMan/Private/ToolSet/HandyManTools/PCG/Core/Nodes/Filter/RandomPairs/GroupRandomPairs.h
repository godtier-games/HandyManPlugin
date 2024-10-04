// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "UObject/Object.h"
#include "GroupRandomPairs.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, ClassGroup = (HandyMan))
class HANDYMAN_API UGroupRandomPairsSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	UGroupRandomPairsSettings();

	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("GroupRandomPairsFromAttributes")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGGroupRandomPairsSettings", "NodeTitle", "Group Random Pairs From Attributes"); }
	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGGroupRandomPairsSettings", "NodeTooltip", "Filter and group points with a random pair. Use this to generate 2 points to create curves."); }
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
	/** The Start Point Attribute Name. This should point to an attribute that already exists on the incoming points */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	FName StartPointAttributeName = NAME_None;
	
	/** The End Points Attribute Name. This should point to an attribute that already exists on the incoming points */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	FName EndPointAttributeName = NAME_None;

	/** The Excess Point Attribute Name. This should point to an attribute that already exists on the incoming points */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	FName ExcessPointAttributeName = NAME_None;

	/** The Attribute you want to update with new data. This should point to an attribute that already exists on the incoming points*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	FName AttributeNameToAdjust = NAME_None;
	
};

class FPCGGroupRandomPairsElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

	// Cache the seed value of the selected points so there are no duplicates
};

