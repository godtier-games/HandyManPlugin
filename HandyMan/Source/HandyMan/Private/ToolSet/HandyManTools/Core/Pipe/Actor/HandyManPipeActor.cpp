// Fill out your copyright notice in the Description page of Project Settings.


#include "HandyManPipeActor.h"

#include "ModelingUtilities/GodtierModelingUtilities.h"



// Sets default values
AHandyManPipeActor::AHandyManPipeActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SplineComponent = CreateDefaultSubobject<USplineComponent>("SplineComponent");
	SplineComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AHandyManPipeActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHandyManPipeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHandyManPipeActor::RebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{
	TargetMesh->Reset();

	FSweepOptions Options;

	Options.TargetMesh = TargetMesh;
	Options.Spline = SplineComponent;
	Options.bFlipOrientation = bFlipOrientation;
	Options.bResampleCurve = true;
	Options.SampleSize = 10;
	
	UGodtierModelingUtilities::SweepGeometryAlongSpline(Options);
	
	Super::RebuildGeneratedMesh(TargetMesh);
}

