// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimToTextureProxyActor.h"


// Sets default values
AAnimToTextureProxyActor::AAnimToTextureProxyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAnimToTextureProxyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAnimToTextureProxyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

