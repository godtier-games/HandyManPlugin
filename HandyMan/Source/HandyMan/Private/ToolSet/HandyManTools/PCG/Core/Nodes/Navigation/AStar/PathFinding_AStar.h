// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "UObject/Object.h"
#include "PathFinding_AStar.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UPathFinding_AStarSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	UPathFinding_AStarSettings();

	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("PathFinding_A_Star")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGFilterBySeedValueSettings", "NodeTitle", "A Star Path Finding"); }
	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGFilterBySeedValueSettings", "NodeTooltip", "Compare the point array to the input value and return it if it matches."); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::PointOps; }
#endif

	virtual FName AdditionalTaskName() const override;

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;
	//~End UPCGSettings interface
	
};

class FPCGAStarPathfindingElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

	// Cache the seed value of the selected points so there are no duplicates
};
