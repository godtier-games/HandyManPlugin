// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "PCGSettings.h"
#include "OrientPointSettings.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, ClassGroup = (HandyMan))
class HANDYMAN_API UOrientPointSettings : public UPCGSettings
{

	GENERATED_BODY()
	
public:
	UOrientPointSettings();

	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("TransformPoints")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGTransformPointsSettings", "NodeTitle", "Orient Points"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::PointOps; }
#endif
	

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return Super::DefaultPointInputPinProperties(); }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override { return Super::DefaultPointOutputPinProperties(); }
	virtual FPCGElementPtr CreateElement() const override;
	virtual bool UseSeed() const override {return true;}
	//~End UPCGSettings interface

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	float MeshDistanceOffset = 0.0f;
};

class FPCGOrientPointsElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#include "Elements/PCGPointProcessingElementBase.h"
#endif
