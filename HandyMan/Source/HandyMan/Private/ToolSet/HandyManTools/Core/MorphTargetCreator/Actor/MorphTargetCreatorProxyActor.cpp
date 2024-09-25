// Fill out your copyright notice in the Description page of Project Settings.


#include "MorphTargetCreatorProxyActor.h"


// Sets default values
AMorphTargetCreatorProxyActor::AMorphTargetCreatorProxyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMorphTargetCreatorProxyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMorphTargetCreatorProxyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

