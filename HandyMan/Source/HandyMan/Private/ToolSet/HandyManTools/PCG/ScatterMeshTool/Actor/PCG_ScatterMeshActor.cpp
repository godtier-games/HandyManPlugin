// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG_ScatterMeshActor.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Helpers/PCGGraphParametersHelpers.h"

// Sets default values
APCG_ScatterMeshActor::APCG_ScatterMeshActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PCG = CreateDefaultSubobject<UPCGComponent>(TEXT("PCG"));
	PCG->bIsComponentPartitioned = false;
	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	DisplayMesh->SetupAttachment(RootComponent);
	DisplayMesh->SetMobility(EComponentMobility::Movable);
	DisplayMesh->SetCollisionProfileName("BlockAll");
	
}

void APCG_ScatterMeshActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (PCG && PCG->GetGraphInstance())
	{
		if (!MeshToScatterOn.ToSoftObjectPath().IsNull())
		{
			DisplayMesh->SetStaticMesh(MeshToScatterOn.LoadSynchronous());
			UPCGGraphParametersHelpers::SetSoftObjectParameter(PCG->GetGraphInstance(), FName("InputMesh"), MeshToScatterOn);
		}
	}
}

void APCG_ScatterMeshActor::TransferMeshMaterials(TArray<UMaterialInterface*> Materials)
{
	if (DisplayMesh)
	{
		for (int i = 0; i < Materials.Num(); i++)
		{
			DisplayMesh->SetMaterial(i, Materials[i]);
		}
	}
	
}

// Called when the game starts or when spawned
void APCG_ScatterMeshActor::BeginPlay()
{
	Super::BeginPlay();
}

