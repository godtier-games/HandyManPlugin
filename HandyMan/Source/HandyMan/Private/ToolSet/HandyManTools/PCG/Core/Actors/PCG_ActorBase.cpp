// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_ActorBase.h"

#include "PCGComponent.h"


// Sets default values
APCG_ActorBase::APCG_ActorBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
}

// Called when the game starts or when spawned
void APCG_ActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APCG_ActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

