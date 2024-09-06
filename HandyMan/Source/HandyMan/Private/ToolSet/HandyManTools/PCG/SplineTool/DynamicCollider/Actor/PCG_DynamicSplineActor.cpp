// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG_DynamicSplineActor.h"

#include "GeometryScript/MeshModelingFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "Metadata/PCGMetadataAttributeTraits.h"
#include "ModelingUtilities/GodtierModelingUtilities.h"


// Sets default values
APCG_DynamicSplineActor::APCG_DynamicSplineActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = DefaultSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneComponent"));
	PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineComponent->SetupAttachment(RootComponent);

	DynamicMeshComponent->CastShadow = false;
	DynamicMeshComponent->CollisionType = CTF_UseComplexAsSimple;
	DynamicMeshComponent->bEnableComplexCollision = true;

	Tags.Add(FName("Rail"));

}

void APCG_DynamicSplineActor::RefreshDynamicCollision()
{
	if (DynamicMeshComponent)
	{
		FVector2D MeshScale = FVector2D(10 * ColliderAdditiveScale.Y, SplineMesh ? SplineMesh->GetExtendedBounds().BoxExtent.Y * ColliderAdditiveScale.X : 0);
		FSimpleCollisionOptions Options;
		Options.Height = 10 * ColliderAdditiveScale.Y;
		Options.Width = SplineMesh ? SplineMesh->GetExtendedBounds().BoxExtent.Y * ColliderAdditiveScale.X : 0;
		Options.ErrorTolerance = 1.f;
		Options.TargetMesh = DynamicMeshComponent->GetDynamicMesh();
		Options.Spline = SplineComponent;
		Options.ZOffset = -1.0; /*SplineMesh ? SplineMesh->GetExtendedBounds().BoxExtent.Z * 2.f : 0.f;*/

		FSweepOptions SweepOptions;
		SweepOptions.SampleSize = SplineComponent->GetSplineLength() / 10;
		SweepOptions.bProjectPointsToSurface = false;
		SweepOptions.ShapeDimensions = MeshScale;
		SweepOptions.ShapeType = ESweepShapeType::Box;
		SweepOptions.TargetMesh = DynamicMeshComponent->GetDynamicMesh();
		SweepOptions.Spline = SplineComponent;
		SweepOptions.bResampleCurve = true;
		SweepOptions.bFlipOrientation = false;
		

		UGodtierModelingUtilities::SweepGeometryAlongSpline(SweepOptions, ESplineCoordinateSpace::World);

		FGeometryScriptSplitNormalsOptions SplitNormalsOptions;
		FGeometryScriptCalculateNormalsOptions CalculateNormalsOptions;
		CalculateNormalsOptions.bAreaWeighted = true;
		UGeometryScriptLibrary_MeshNormalsFunctions::ComputeSplitNormals(DynamicMeshComponent->GetDynamicMesh(), SplitNormalsOptions, CalculateNormalsOptions);

		if (!SplineMesh.IsNull())
		{
			DynamicMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, SplineMesh.LoadSynchronous()->GetBounds().BoxExtent.Z + ColliderZOffset));
		}
		
	}
}

void APCG_DynamicSplineActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	RefreshDynamicCollision();
	
}

// Called when the game starts or when spawned
void APCG_DynamicSplineActor::BeginPlay()
{
	Super::BeginPlay();

	DynamicMeshComponent->SetHiddenInGame(true);
	
}

// Called every frame
void APCG_DynamicSplineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APCG_DynamicSplineActor::SetSplinePoints(const TArray<FTransform> Points)
{
	if (!SplineComponent)
	{
		return;
		
	}
	
	SplineComponent->ClearSplinePoints(true);
	for (int i = 0; i < Points.Num(); i++)
	{
		SplineComponent->AddSplineWorldPoint(Points[i].GetLocation());

		if (Points.Num() > i + 1 /*&& bAimMeshAtNextPoint*/)
		{
			FVector Tangent = Points.IsValidIndex(i + 1) ?  Points[i + 1].GetLocation() - Points[i].GetLocation() : FVector::ZeroVector;
			SplineComponent->SetRotationAtSplinePoint(i, Tangent.ToOrientationRotator(), ESplineCoordinateSpace::World);
		}
		
	}

	for (int i = 0; i < SplineComponent->GetNumberOfSplinePoints(); i++)
	{
		if(i == 0 || i == SplineComponent->GetNumberOfSplinePoints() - 1)
		{
			SplineComponent->SetSplinePointType(i, ESplinePointType::Linear);
		}
		else
		{
			SplineComponent->SetSplinePointType(i, SplinePointType);
		}
	}
	

	if (PCGComponent)
	{
		PCGComponent->NotifyPropertiesChangedFromBlueprint();
	}

	RefreshDynamicCollision();
}

void APCG_DynamicSplineActor::SetSplinePointType(const ESplinePointType::Type PointType)
{
	SplinePointType = PointType;
	if (SplineComponent)
	{
		for (int i = 0; i < SplineComponent->GetNumberOfSplinePoints(); i++)
		{
			SplineComponent->SetSplinePointType(i, PointType);
		}

		PCGComponent->NotifyPropertiesChangedFromBlueprint();
	}

	RerunConstructionScripts();
};

#if WITH_EDITOR



void APCG_DynamicSplineActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, SplinePointType) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, bAimMeshAtNextPoint) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, bEnableRandomRotation) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MinRandomRotation) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MaxRandomRotation) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MeshOffsetDistance) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, SplineMesh) || 
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, ColliderZOffset) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, SplinePointType))
	{
		RerunConstructionScripts();

		if (PCGComponent)
		{
			PCGComponent->CleanupLocal(true, true);
			PCGComponent->NotifyPropertiesChangedFromBlueprint();
		}
	
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, SplineMesh) && !SplineMesh.IsNull() && DynamicMeshComponent)
	{
		DynamicMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, SplineMesh.LoadSynchronous()->GetBounds().BoxExtent.Z + ColliderZOffset));
	}
}


#endif

