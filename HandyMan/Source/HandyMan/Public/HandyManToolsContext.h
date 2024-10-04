// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdModeInteractiveToolsContext.h"
#include "HandyManToolsContext.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManToolsContext : public UEdModeInteractiveToolsContext
{
	GENERATED_BODY()

public:

	virtual void DeactivateAllActiveTools(EToolShutdownType ShutdownType) override;
};
