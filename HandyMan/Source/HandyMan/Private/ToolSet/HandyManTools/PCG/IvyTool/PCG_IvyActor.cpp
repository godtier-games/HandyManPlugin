// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/PCG/IvyTool/PCG_IvyActor.h"

#include "PCGComponent.h"


// Sets default values
APCG_IvyActor::APCG_IvyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PCG = CreateDefaultSubobject<UPCGComponent>(TEXT("PCG"));
}

// Called when the game starts or when spawned
void APCG_IvyActor::BeginPlay()
{
	Super::BeginPlay();
}


