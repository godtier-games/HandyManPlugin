// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "UObject/Object.h"
#include "OrientPointTowardsOrigin.generated.h"

UENUM(BlueprintType)
enum class EAxisToKeep : uint8
{
	None UMETA(Hidden),
	Roll,
	Pitch,
	Yaw
};


/**
 * 
 */
UCLASS()
class HANDYMAN_API UOrientPointTowardsOriginSettings : public UPCGSettings
{

	GENERATED_BODY()
	
public:
	UOrientPointTowardsOriginSettings();

	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("OrientPointsTowardsOrigin")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGTransformPointsSettings", "NodeTitle", "Orient Points Towards Origin"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::PointOps; }
#endif
	

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override { return Super::DefaultPointOutputPinProperties(); }
	virtual FPCGElementPtr CreateElement() const override;
	virtual bool UseSeed() const override {return true;}
	//~End UPCGSettings interface

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	TMap<EAxisToKeep, bool> AxisToKeep;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	bool bDebugOrientations = false;
};

class FPCGOrientPointsToOriginElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#include "Elements/PCGPointProcessingElementBase.h"
#endif

