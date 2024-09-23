// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "UObject/Object.h"
#include "PCGWorldQuery_SphereTrace.generated.h"

UENUM(BlueprintType)
enum class ETransformResolutionType : uint8
{
	/* Uses the first hit location/rotation in the TArray<FHitResult> */
	First,
	
	/* Uses the last hit location/rotation in the TArray<FHitResult> */
	Last,

	/* Uses the random location/rotation in the TArray<FHitResult> */
	Random
};

/**
 * 
 */
UCLASS()
class HANDYMAN_API UPCGWorldQuery_SphereTraceSettings : public UPCGSettings
{
	GENERATED_BODY()
public:
	UPCGWorldQuery_SphereTraceSettings();

	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("AttachPointsToNearbySurfaces")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGFilterBySeedValueSettings", "NodeTitle", "Attach Points To Nearby Surfaces"); }
	virtual FText GetNodeTooltipText() const override
	{
		return NSLOCTEXT("PCGFilterBySeedValueSettings", "NodeTooltip", "Sphere trace out into the world from a points original location and attach it to the hit location ");
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
	/** The seed value we want the matching point to have */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	bool bInheritSurfaceRotation = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	bool bDebugQueries = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	float SphereRadius = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	float TraceHeightOffset = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	TArray<TEnumAsByte<ECollisionChannel>> ChannelsToTrace;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	ETransformResolutionType TransformResolution = ETransformResolutionType::First;

	
};

class FPCGSphereTraceElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

	bool Trace(const UWorld* World, const FVector& Start, const FVector& End,  const TArray<TEnumAsByte<ECollisionChannel>>& Types, const float& Radius, TArray<FHitResult>& OutHits) const;

	// Cache the seed value of the selected points so there are no duplicates
};
