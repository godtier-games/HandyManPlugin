// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_DynamicMeshActor_Runtime.h"


// Sets default values
APCG_DynamicMeshActor_Runtime::APCG_DynamicMeshActor_Runtime()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (DynamicMeshComponent)
	{
		MeshObjectChangedHandle = DynamicMeshComponent->GetDynamicMesh()->OnMeshChanged().AddUObject(this, &APCG_DynamicMeshActor_Runtime::OnMeshObjectChanged);
	}
}

// Called when the game starts or when spawned
void APCG_DynamicMeshActor_Runtime::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APCG_DynamicMeshActor_Runtime::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APCG_DynamicMeshActor_Runtime::SetCollisionProfileByName(const FName ProfileName)
{
	CollisionProfileName = ProfileName;
	DynamicMeshComponent->SetCollisionProfileName(CollisionProfileName);
}

void APCG_DynamicMeshActor_Runtime::OnMeshObjectChanged(UDynamicMesh* ChangedMeshObject, FDynamicMeshChangeInfo ChangeInfo)
{
	// Update the collision settings for this mesh
	if (DynamicMeshComponent)
	{
		DynamicMeshComponent->UpdateCollision();
		DynamicMeshComponent->SetCollisionProfileName(CollisionProfileName);
	}
	
}

