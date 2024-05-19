// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/PCG/IvyTool/PCG_IvyActor.h"

#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Helpers/PCGGraphParametersHelpers.h"


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
		}

		UPCGGraphParametersHelpers::SetSoftObjectParameter(PCG->GetGraphInstance(), FName("InputMesh"), MeshToGiveVines);
		UPCGGraphParametersHelpers::SetSoftObjectParameter(PCG->GetGraphInstance(), FName("VineMesh"), VineMesh);

		PCG->NotifyPropertiesChangedFromBlueprint();
	}
}

void APCG_IvyActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

#if WITH_EDITOR
  void APCG_IvyActor::GenerateVines(const FPCGDataCollection& Data)
{

	TArray<USplineComponent*> Components ;
	GetComponents(USplineComponent::StaticClass(), Components);

	TArray<FTempSplinePoint> SplinePoints;

	int32 NumberOfSplineMeshesNeeded = 0;

	for (auto Item : Components)
	{
		for (int i = 0; i < Item->GetNumberOfSplinePoints(); i++)
		{
			NumberOfSplineMeshesNeeded++;
			
			FVector StartPos, StartTangent, EndPos, EndTangent;
			Item->GetLocationAndTangentAtSplinePoint(i, StartPos, StartTangent, ESplineCoordinateSpace::World);
			Item->GetLocationAndTangentAtSplinePoint(Components.IsValidIndex(i + 1) ? i + 1 : i, EndPos, EndTangent, ESplineCoordinateSpace::World);
			SplinePoints.Add(FTempSplinePoint(StartPos, StartTangent, EndPos, EndTangent));
		}
	}


	if (NumberOfSplineMeshesNeeded != Vines.Num())
	{
		// Check if we are over or under
		if (NumberOfSplineMeshesNeeded > Vines.Num())
		{
			// We need to add more
			for (int i = Vines.Num(); i < NumberOfSplineMeshesNeeded; i++)
			{
				auto NewVine = NewObject<USplineMeshComponent>(this);
				NewVine->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				NewVine->SetMobility(EComponentMobility::Movable);
				NewVine->SetStaticMesh(VineMesh.LoadSynchronous());
				NewVine->SetCollisionProfileName("NoCollision");
				Vines.Add(NewVine);
			}
		}
		else
		{
			// We need to remove some
			for (int i = Vines.Num() - 1; i >= NumberOfSplineMeshesNeeded; i--)
			{
				Vines[i]->DestroyComponent();
				Vines.RemoveAt(i);
			}
		}
	}

	for (int i = 0; i < SplinePoints.Num(); i++)
	{
		Vines[i]->SetStartAndEnd(SplinePoints[i].StartLocation, SplinePoints[i].StartTangent, SplinePoints[i].EndLocation, SplinePoints[i].EndTangent);
		Vines[i]->RegisterComponent();
	} 

}
#endif


// Called when the game starts or when spawned
void APCG_IvyActor::BeginPlay()
{
	Super::BeginPlay();
}

