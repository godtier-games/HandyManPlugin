// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG_IvyActor.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Components/SplineComponent.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "Helpers/PCGGraphParametersHelpers.h"
#include "ModelingUtilities/GodtierModelingUtilities.h"


// Sets default values
APCG_IvyActor::APCG_IvyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PCG = CreateDefaultSubobject<UPCGComponent>(TEXT("PCG"));
	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	DisplayMesh->SetupAttachment(RootComponent);
	DisplayMesh->SetMobility(EComponentMobility::Movable);
	DisplayMesh->SetCollisionProfileName("BlockAll");
	
}

void APCG_IvyActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (PCG && PCG->GetGraphInstance())
	{
		if (!MeshToGiveVines.ToSoftObjectPath().IsNull())
		{
			DisplayMesh->SetStaticMesh(MeshToGiveVines.LoadSynchronous());
			UPCGGraphParametersHelpers::SetSoftObjectParameter(PCG->GetGraphInstance(), FName("InputMesh"), MeshToGiveVines);
		}
	}
}

void APCG_IvyActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}


  void APCG_IvyActor::GenerateVines(const FPCGDataCollection& Data)
{
	MarkForMeshRebuild(true);
}


void APCG_IvyActor::SetVineThickness(const float Thickness)
{
	VineThickness = Thickness;
	if (PCG)
	{
		PCG->NotifyPropertiesChangedFromBlueprint();
	}
	/*for (int i = 0; i < Vines.Num(); i++)
	{
		Vines[i]->SetStartScale(FVector2D(Thickness, Thickness));
		Vines[i]->SetEndScale(FVector2D(Thickness, Thickness));
	}*/
}

void APCG_IvyActor::TransferMeshMaterials(TArray<UMaterialInterface*> Materials)
{
	if (DisplayMesh)
	{
		for (int i = 0; i < Materials.Num(); i++)
		{
			DisplayMesh->SetMaterial(i, Materials[i]);
		}
	}
	
}

void APCG_IvyActor::RebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{
	TargetMesh->Reset();
	UDynamicMesh* CombinedSplinesMesh = nullptr;
	auto TempMesh = AllocateComputeMesh();
	
	TArray<USplineComponent*> Components ;
	GetComponents(USplineComponent::StaticClass(), Components);

	
	FSweepOptions SweepOptions;
	SweepOptions.TargetMesh = TempMesh;
	SweepOptions.ShapeType = ESweepShapeType::Circle;
	SweepOptions.ShapeRadius = VineThickness;
	SweepOptions.ShapeSegments = 8;

	for (auto Item : Components)
	{
		SweepOptions.Spline = Item;
		CombinedSplinesMesh = UGodtierModelingUtilities::SweepGeometryAlongSpline(SweepOptions, ESplineCoordinateSpace::Local, nullptr);
	}

	UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(TargetMesh, CombinedSplinesMesh, FTransform::Identity);
	if (!VineMaterial.ToString().IsEmpty())
	{
		DynamicMeshComponent->SetMaterial(0, VineMaterial.LoadSynchronous());
	}
	
	ReleaseAllComputeMeshes();
}


// Called when the game starts or when spawned
void APCG_IvyActor::BeginPlay()
{
	Super::BeginPlay();
}

