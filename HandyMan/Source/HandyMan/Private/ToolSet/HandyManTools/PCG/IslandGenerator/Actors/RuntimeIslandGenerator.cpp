// Fill out your copyright notice in the Description page of Project Settings.


#include "RuntimeIslandGenerator.h"

#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/MeshDeformFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshSubdivideFunctions.h"
#include "GeometryScript/MeshUVFunctions.h"
#include "GeometryScript/MeshVoxelFunctions.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialParameterCollectionInstance.h"


// Sets default values
ARuntimeIslandGenerator::ARuntimeIslandGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ARuntimeIslandGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARuntimeIslandGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bShouldGenerateOnConstruction)
	{
		GenerateIsland();
	}
}

void ARuntimeIslandGenerator::SetShouldGenerateOnConstruction(const bool ShouldGenerate)
{
	bShouldGenerateOnConstruction = ShouldGenerate;
	GenerateIsland();
}

void ARuntimeIslandGenerator::SetRandomSeed(const int32 InRandomSeed)
{
	RandomSeed = InRandomSeed;
	GenerateIsland();
}

void ARuntimeIslandGenerator::SetIslandChunkCount(const int32 InCount)
{
	IslandChunks = InCount;
	GenerateIsland();
}

void ARuntimeIslandGenerator::SetIslandBounds(const FVector2D& InBounds)
{
	IslandBounds = InBounds;
	GenerateIsland();
}

void ARuntimeIslandGenerator::SetMaxSpawnArea(const float SpawnArea)
{
	MaxSpawnArea = SpawnArea;
	GenerateIsland();
}

void ARuntimeIslandGenerator::SetIslandDepth(const float Depth)
{
	IslandDepth = Depth;
	GenerateIsland();
}

void ARuntimeIslandGenerator::SetShouldFlattenIsland(const bool ShouldFlatten)
{
	bShouldFlattenIsland = ShouldFlatten;
	GenerateIsland();
}

void ARuntimeIslandGenerator::SetMaterialParameterCollection(const TObjectPtr<UMaterialParameterCollection>& NewCollection)
{
	MaterialParameterCollection = NewCollection;
	GenerateIsland();
}

void ARuntimeIslandGenerator::SetGrassColor(const FLinearColor& Color)
{
	GrassColor = Color;
	GenerateIsland();
}

// Called every frame
void ARuntimeIslandGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARuntimeIslandGenerator::GenerateIsland()
{
	GetDynamicMeshComponent()->GetDynamicMesh()->Reset();
	auto OutputMesh = GetDynamicMeshComponent()->GetDynamicMesh();

	FRandomStream RandomStream(RandomSeed);
	TArray<FVector> SpawnLocations;

	for (int i = 0; i < IslandChunks; ++i)
	{
		const FVector RandUnitVector = UKismetMathLibrary::RandomUnitVectorFromStream(RandomStream);
		FVector RandLocation = RandUnitVector * FVector(MaxSpawnArea * 0.5f);
		RandLocation.Z = GetActorLocation().Z + IslandDepth;
		
		float Radius = RandomStream.FRandRange(IslandBounds.X, IslandBounds.Y);
		
		SpawnLocations.Add(RandLocation);

		const FTransform MeshTransform = FTransform(FRotator(0.0f, 0.0f, 0.0f), RandLocation, FVector(1.0f, 1.0f, 1.0f));

		auto Mesh = UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCone
		(OutputMesh, FGeometryScriptPrimitiveOptions(), MeshTransform, Radius, Radius * 0.25f, 1300.f, 12, 1);

		// TODO : Spawn some markers ??
	}

	// For some reason a box is appened to the mesh.
	const FTransform AppendTransform = FTransform(FRotator(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, GetActorLocation().Z + IslandDepth), FVector(1.0f, 1.0f, 1.0f));
	const float Dimension = MaxSpawnArea + 10000.0f;
	auto MeshWithBox = UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox
	(OutputMesh, FGeometryScriptPrimitiveOptions(), AppendTransform, Dimension, Dimension, FMath::Abs(IslandDepth * 0.5f));

	FGeometryScript3DGridParameters GridParameters;
	GridParameters.SizeMethod = EGeometryScriptGridSizingMethod::GridResolution;
	GridParameters.GridCellSize = 0.25f;
	GridParameters.GridResolution = PlatformSwitch(64, 32);
	FGeometryScriptSolidifyOptions SolidifyOptions;
	SolidifyOptions.SurfaceSearchSteps = 64;
	SolidifyOptions.ExtendBounds = 0.f;
	SolidifyOptions.GridParameters = GridParameters;
	
	auto VoxelMesh = UGeometryScriptLibrary_MeshVoxelFunctions::ApplyMeshSolidify(MeshWithBox, SolidifyOptions);


	auto MeshWithNormals = UGeometryScriptLibrary_MeshNormalsFunctions::SetPerVertexNormals(VoxelMesh);

	FGeometryScriptIterativeMeshSmoothingOptions IterativeSmoothingOptions;
	IterativeSmoothingOptions.NumIterations = 6;
	
	auto SmoothedMesh = UGeometryScriptLibrary_MeshDeformFunctions::ApplyIterativeSmoothingToMesh(MeshWithNormals, FGeometryScriptMeshSelection(), IterativeSmoothingOptions);

	auto TessellatedMesh = UGeometryScriptLibrary_MeshSubdivideFunctions::ApplyPNTessellation(SmoothedMesh, FGeometryScriptPNTessellateOptions(), PlatformSwitch(0, 2));

	const FTransform CutTransform = FTransform(FRotator(0.0f, 0.0f, 180.0f), FVector(0.0f, 0.0f, GetActorLocation().Z + IslandDepth * 0.5), FVector(1.0f, 1.0f, 1.0f));
	FGeometryScriptMeshPlaneCutOptions PlaneCutOptions;
	PlaneCutOptions.bFillHoles = false;
	PlaneCutOptions.bFillSpans = false;
	PlaneCutOptions.bFlipCutSide = false;
	OutputMesh = UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(TessellatedMesh, CutTransform, PlaneCutOptions);


	if (bShouldFlattenIsland)
	{
		auto FlattenedMesh = UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(OutputMesh, FTransform::Identity, FGeometryScriptMeshPlaneCutOptions());
		UGeometryScriptLibrary_MeshUVFunctions::SetMeshUVsFromPlanarProjection
		(FlattenedMesh, 0, FTransform(FRotator::ZeroRotator, FVector::Zero(), FVector(100.f)), FGeometryScriptMeshSelection());
	}

	ReleaseAllComputeMeshes();

	AddActorWorldOffset(FVector(0,0,0.05));
	
	if (MaterialParameterCollection)
	{
		if (UMaterialParameterCollectionInstance* MaterialParameterCollectionInstance = GetWorld()->GetParameterCollectionInstance(MaterialParameterCollection))
		{
			MaterialParameterCollectionInstance->SetVectorParameterValue(FName("GrassColour"), GrassColor);
		}	
	}
	
}

int32 ARuntimeIslandGenerator::PlatformSwitch(const int32 LowEnd, const int32 HighEnd) const
{
	const FString Platform = UGameplayStatics::GetPlatformName();

	if (Platform.Equals("Android") || Platform.Equals("Switch") || Platform.Equals("IOS"))
	{
		return LowEnd;
	}

	return HighEnd;
}

#if WITH_EDITOR
void ARuntimeIslandGenerator::RegenerateIsland()
{
	GenerateIsland();
}

void ARuntimeIslandGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, bShouldFlattenIsland))
	{
		RerunConstructionScripts();
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MaterialParameterCollection))
	{
		RerunConstructionScripts();
	}
	
	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, IslandBounds))
	{
		RerunConstructionScripts();
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, IslandChunks))
	{
		RerunConstructionScripts();
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MaxSpawnArea))
	{
		RerunConstructionScripts();
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, IslandDepth))
	{
		RerunConstructionScripts();
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, RandomSeed))
	{
		RerunConstructionScripts();
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, GrassColor))
	{
		RerunConstructionScripts();
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, bShouldGenerateOnConstruction))
	{
		if (bShouldGenerateOnConstruction)
		{
			RerunConstructionScripts();
		}
	}

	

	
}

#endif
