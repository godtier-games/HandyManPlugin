﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCG_ActorBase.generated.h"

UCLASS()
class HANDYMAN_API APCG_ActorBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APCG_ActorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandyMan")
	TObjectPtr<class USceneComponent> DefaultSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandyMan")
	TObjectPtr<class UPCGComponent> PCGComponent;
};
