// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "CullFromTraceSettings.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UCullFromTraceSettings : public UPCGSettings
{
	GENERATED_BODY()
public:
	UCullFromTraceSettings();

	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("CullFromSphereTrace")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("UCullFromTraceSettings", "NodeTitle", "Cull Colliding Points"); }
	virtual FText GetNodeTooltipText() const override
	{
		return NSLOCTEXT("CullFromTraceSettings", "NodeTooltip", "Remove any points that are located between 2 walls");
	}
	
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::PointOps; }
#endif


protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;
	virtual bool UseSeed() const override {return true;}
	//~End UPCGSettings interface

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	float SphereRadius = 100.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	TArray<TEnumAsByte<ECollisionChannel>> ChannelsToTrace;
	
};

class FPCGCullPointsElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

	bool Trace(const UWorld* World, const FVector& Start, const FVector& End,  const TArray<TEnumAsByte<ECollisionChannel>>& Types, const float& Radius, TArray<FHitResult>& OutHits) const;

	// Cache the seed value of the selected points so there are no duplicates
};

