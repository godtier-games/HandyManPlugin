﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGComponent.h"
#include "GameFramework/Actor.h"
#include "ToolSet/HandyManTools/PCG/Core/Interface/PCGToolInterface.h"
#include "PCG_ActorBase.generated.h"



UCLASS()
class HANDYMAN_API APCG_ActorBase : public AActor, public IPCGToolInterface
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

	virtual UPCGComponent* GetPCGComponent() const override { return nullptr; }

};
