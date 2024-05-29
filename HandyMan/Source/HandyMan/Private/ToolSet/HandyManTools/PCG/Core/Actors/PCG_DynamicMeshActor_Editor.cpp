// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_DynamicMeshActor_Editor.h"


// Sets default values
APCG_DynamicMeshActor_Editor::APCG_DynamicMeshActor_Editor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APCG_DynamicMeshActor_Editor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APCG_DynamicMeshActor_Editor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

