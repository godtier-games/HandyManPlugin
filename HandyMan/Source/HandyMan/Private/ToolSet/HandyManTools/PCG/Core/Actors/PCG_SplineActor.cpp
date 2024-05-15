// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_SplineActor.h"

#include "PCGComponent.h"
#include "Components/SplineComponent.h"


// Sets default values
APCG_SplineActor::APCG_SplineActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = DefaultSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneComponent"));
	PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APCG_SplineActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APCG_SplineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APCG_SplineActor::SetSplinePoints(const TArray<FTransform>& Points)
{
	if (!SplineComponent)
	{
		return;
		
	}
	
	SplineComponent->ClearSplinePoints(true);
	for (int i = 0; i < Points.Num(); i++)
	{
		SplineComponent->AddSplineWorldPoint(Points[i].GetLocation());

		if (Points.Num() > i + 1 && bAimMeshAtNextPoint)
		{
			FVector Tangent = Points[i + 1].GetLocation() - Points[i].GetLocation();
			SplineComponent->SetRotationAtSplinePoint(i, Tangent.ToOrientationRotator(), ESplineCoordinateSpace::World);
		}
		
	}

	for (int i = 0; i < SplineComponent->GetNumberOfSplinePoints(); i++)
	{
		SplineComponent->SetSplinePointType(i, SplinePointType);
	}
	

	if (PCGComponent)
	{
		PCGComponent->NotifyPropertiesChangedFromBlueprint();
	}
}

void APCG_SplineActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, SplinePointType) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, bAimMeshAtNextPoint) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, bEnableRandomRotation) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MinRandomRotation) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MaxRandomRotation) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MeshOffsetDistance) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MeshHeightRange) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, SplineMesh))
	{
		RerunConstructionScripts();
		if (PCGComponent)
		{
			PCGComponent->NotifyPropertiesChangedFromBlueprint();
		}
	}
}

