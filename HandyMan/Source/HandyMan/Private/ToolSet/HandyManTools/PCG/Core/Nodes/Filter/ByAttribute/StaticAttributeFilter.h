// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGElement.h"
#include "PCGSettings.h"
#include "Elements/PCGAttributeFilter.h"
#include "UObject/Object.h"
#include "StaticAttributeFilter.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, ClassGroup = (HandyMan))
class HANDYMAN_API UStaticAttributeFilter : public UPCGSettings
{
	GENERATED_BODY()
public:
	UStaticAttributeFilter();

	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("NumberAttributeFilter")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGAttributeFilteringElement", "NodeTitleRange", "Number Attribute Filter"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Filter; }
	virtual bool HasDynamicPins() const override { return true; }
#endif
	virtual FString GetAdditionalTitleInformation() const override;
	virtual EPCGDataType GetCurrentPinTypes(const UPCGPin* InPin) const override;

protected:
	virtual FPCGElementPtr CreateElement() const override;
	//~End UPCGSettings interface

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	EPCGAttributeFilterOperator Operator = EPCGAttributeFilterOperator::Greater;

	/** Target property/attribute related properties */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	FPCGAttributePropertyInputSelector TargetAttribute;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (EditCondition = "bAllowsTypeChange", EditConditionHides, HideEditConditionToggle, ValidEnumValues = "Float, Double, Integer32, Integer64"))
	EPCGMetadataTypes Type = EPCGMetadataTypes::Float;

	// All different types
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	float TargetValue = 0.0f;

	// Hidden value to indicate that Spatial -> Point deprecation is on where pins are not explicitly points.
	UPROPERTY()
	bool bHasSpatialToPointDeprecation = false;
};

class FPCGNumberAttributeFilterElementBase : public IPCGElement
{
protected:
	bool DoFiltering(FPCGContext* Context, EPCGAttributeFilterOperator InOperation, const FPCGAttributePropertyInputSelector& TargetAttribute, bool bHasSpatialToPointDeprecation, const FPCGAttributeFilterThresholdSettings& FirstThreshold, const FPCGAttributeFilterThresholdSettings* SecondThreshold = nullptr) const;
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

/* TODO: Add child classes that add static filters for all types
class FPCGAttributeFilterElement : public FPCGNumberAttributeFilterElementBase
{

};

class FPCGAttributeFilterRangeElement : public FPCGAttributeFilterElementBase
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};
*/

