// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_ActorBase.h"
#include "PCG_IvyActor.generated.h"

class UPCGComponent;

UCLASS()
class HANDYMAN_API APCG_IvyActor : public APCG_ActorBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APCG_IvyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	virtual UPCGComponent* GetPCGComponent() const override {return PCG;}

protected:

	UPROPERTY(BlueprintReadWrite, Category="Inputs")
	TArray<AActor*> TargetActors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPCGComponent> PCG;
};
