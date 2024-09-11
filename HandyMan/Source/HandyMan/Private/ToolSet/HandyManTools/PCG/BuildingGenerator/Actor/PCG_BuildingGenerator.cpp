// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG_BuildingGenerator.h"
#include "Engine/StaticMeshActor.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshModelingFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "GeometryScript/PolyPathFunctions.h"
#include "ModelingUtilities/GodtierModelingUtilities.h"
#include "PolyPathUtilities/GodtierPolyPathUtilities.h"


// Sets default values
APCG_BuildingGenerator::APCG_BuildingGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PCG = CreateDefaultSubobject<UPCGComponent>(TEXT("PCG"));
	
}

// Called when the game starts or when spawned
void APCG_BuildingGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

void APCG_BuildingGenerator::RebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{
	GenerateExteriorWalls(TargetMesh);
	GenerateRoofMesh(TargetMesh);
	GenerateFloorMeshes(TargetMesh);
}

void APCG_BuildingGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void APCG_BuildingGenerator::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void APCG_BuildingGenerator::CacheInputActor(AActor* InputActor)
{
	if (ADynamicMeshActor* MeshActor_Dynamic = Cast<ADynamicMeshActor>(InputActor))
	{
		OriginalMesh = MeshActor_Dynamic->GetDynamicMeshComponent()->GetDynamicMesh();
		OriginalActor = MeshActor_Dynamic;

		FVector Origin;
		FVector Extent;
		MeshActor_Dynamic->GetActorBounds(false, Origin, Extent);
		BuildingHeight = Extent.Z * 2;

		DesiredFloorHeight = BuildingHeight / NumberOfFloors;
	}
	else if (AStaticMeshActor* MeshActor_Static = Cast<AStaticMeshActor>(InputActor))
	{
		EGeometryScriptOutcomePins Outcome;
		OriginalMesh = UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromStaticMeshV2
		(
			MeshActor_Static->GetStaticMeshComponent()->GetStaticMesh(),
			OriginalMesh,
			FGeometryScriptCopyMeshFromAssetOptions(),
			FGeometryScriptMeshReadLOD(),
			Outcome
		);

		OriginalActor = MeshActor_Static;

		FVector Origin;
		FVector Extent;
		MeshActor_Static->GetActorBounds(false, Origin, Extent);
		BuildingHeight = Extent.Z * 2;

		DesiredFloorHeight = BuildingHeight / NumberOfFloors;
		
	}
	
	TArray<FVector> NormalDirections;
	NormalDirections.Add(FVector(0, 0, 1));
	NormalDirections.Add(FVector(0, 0, -1));
	
	auto PolyPaths = UGodtierPolyPathUtilities::CreatePolyPathFromPlanarFaces(OriginalMesh, NormalDirections, nullptr);
	CreateSplinesFromPolyPaths(PolyPaths);

	
}

void APCG_BuildingGenerator::SetFloorMaterial(const int32 Floor, const TSoftObjectPtr<UMaterialInterface> Material)
{
	if (bUseConsistentFloorMaterial)
	{
		FloorMaterial = Material;
	}
	else
	{
		FloorMaterialMap.Add(Floor, Material);
	}
}

void APCG_BuildingGenerator::CreateSplinesFromPolyPaths(const TArray<FGeometryScriptPolyPath> Paths)
{
	FloorPolyPaths = Paths;

	TArray<FGeometryScriptPolyPath> TempPaths = Paths;

	// get the spline with the lowest Z position to determine the floor the spline is on.
	TArray<float> ZValues;
	for (const auto Path : Paths)
	{
		if (Path.Path.IsValid())
		{
			ZValues.Add(Path.Path->GetData()[0].Z);
		}
	}

	int32 CurrentFloor = 0;

	int32 ExpectedNumberOfSplines = Paths.Num();


	while (ExpectedNumberOfSplines > 0)
	{
		for (int i = 0; i < TempPaths.Num(); ++i)
		{
			auto Path = TempPaths[i];
			
			if (!Path.Path.IsValid())
			{
				ExpectedNumberOfSplines--;
				continue;
			}
		
			int32 IndexOfMaxValue;
			const float MinValue = FMath::Min(ZValues, &IndexOfMaxValue);

			if (!FMath::IsNearlyEqual(Path.Path->GetData()[0].Z, MinValue)) continue;

			TArray<FVector> PathArray;
			UGeometryScriptLibrary_PolyPathFunctions::ConvertPolyPathToArray(Path, PathArray);

			// Create a spline component and add it to the map. If a spline component already exists for that floor just set its points to the new path.
			if (FloorSplines.Contains(CurrentFloor))
			{
				FloorSplines[CurrentFloor]->SetSplinePoints(PathArray, ESplineCoordinateSpace::Local, true);
				ExpectedNumberOfSplines--;
				CurrentFloor++;
				ZValues.RemoveAtSwap(IndexOfMaxValue);
			}
			else
			{
				USplineComponent* Spline = NewObject<USplineComponent>(this);
				Spline->SetSplinePoints(PathArray, ESplineCoordinateSpace::Local, true);
				Spline->ComponentTags.Add(FName(*FString::Printf(TEXT("Floor_%d"), CurrentFloor)));

				Spline->RegisterComponent();
				AddInstanceComponent(Spline);
				Spline->AttachToComponent(GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));

				FloorSplines.Add(CurrentFloor, Spline);
				ExpectedNumberOfSplines--;
				TempPaths.RemoveAtSwap(i);
				ZValues.RemoveAtSwap(IndexOfMaxValue);
				CurrentFloor++;
			}
		}
	}

	for (const auto Floor : FloorSplines)
	{
		for (int i = 0; i < Floor.Value->GetNumberOfSplinePoints(); ++i)
		{
			Floor.Value->SetSplinePointType(i, ESplinePointType::Linear, false);
		}
		Floor.Value->SetClosedLoop(true);
	}

	GenerateExteriorWalls(nullptr);

	if (!bHasOpenRoof)
	{
		GenerateRoofMesh(GetDynamicMeshComponent()->GetDynamicMesh());
	}
	
	GenerateFloorMeshes(GetDynamicMeshComponent()->GetDynamicMesh());
	
	ForceCookPCG();
}

void APCG_BuildingGenerator::GenerateRoofMesh(UDynamicMesh* TargetMesh)
{
	if (!TargetMesh)
	{
		return;
	}

	auto RoofMesh = UGodtierModelingUtilities::GenerateMeshFromPlanarFace(this, OriginalActor);

	FGeometryScriptMeshSelection Selection;
	UGeometryScriptLibrary_MeshSelectionFunctions::CreateSelectAllMeshSelection(RoofMesh, Selection);

	FGeometryScriptMeshLinearExtrudeOptions ExtrudeOptions;
	
	ExtrudeOptions.Direction = FVector(0, 0, -1);
	ExtrudeOptions.Distance = WallThickness;
	ExtrudeOptions.AreaMode = EGeometryScriptPolyOperationArea::EntireSelection;
	
	RoofMesh = UGeometryScriptLibrary_MeshModelingFunctions::ApplyMeshLinearExtrudeFaces(RoofMesh, ExtrudeOptions, Selection);

	UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(TargetMesh, RoofMesh, FTransform::Identity);

	ReleaseAllComputeMeshes();
	
}

void APCG_BuildingGenerator::GenerateFloorMeshes(UDynamicMesh* TargetMesh)
{
	if (!TargetMesh)
	{
		return;
	}
}

void APCG_BuildingGenerator::GenerateExteriorWalls(UDynamicMesh* TargetMesh)
{
	if (TargetMesh)
	{
		TargetMesh->Reset();
	}
	else
	{
		GetDynamicMeshComponent()->GetDynamicMesh()->Reset();
	}
	
	UDynamicMesh* CombinedSplinesMesh = nullptr;
	auto TempMesh = AllocateComputeMesh();
	
	if (FloorSplines.Contains(0))
	{
		FSimpleCollisionOptions SweepOptions;
		SweepOptions.TargetMesh = TempMesh;
		SweepOptions.Spline = FloorSplines[0];
		SweepOptions.Height = BuildingHeight;
		SweepOptions.Width = WallThickness;
		SweepOptions.bOffsetFromCenter = false;
		CombinedSplinesMesh = UGodtierModelingUtilities::GenerateCollisionGeometryAlongSpline(SweepOptions, ESplineCoordinateSpace::Local, nullptr);
	}

	UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(TargetMesh ? TargetMesh : GetDynamicMeshComponent()->GetDynamicMesh(), CombinedSplinesMesh, FTransform::Identity);
	if (!BuildingMaterial.ToString().IsEmpty())
	{
		DynamicMeshComponent->SetMaterial(0, BuildingMaterial.LoadSynchronous());
	}
	
}


void APCG_BuildingGenerator::ForceCookPCG()
{
	PCG->CleanupLocalImmediate(true, true);
	PCG->NotifyPropertiesChangedFromBlueprint();
}

void APCG_BuildingGenerator::SetNumberOfFloors(const int32 NewFloorCount)
{
	NumberOfFloors = NewFloorCount;

	// Technically this should get the input meshes roof mesh, Move it up by the height of the floor and then convert that new mesh to a poly path
	// We then want to create the splines again from the poly paths;
	ForceCookPCG();
}

void APCG_BuildingGenerator::SetHasOpenRoof(const bool HasOpenRoof)
{
    bHasOpenRoof = HasOpenRoof;
    ForceCookPCG();
}

void APCG_BuildingGenerator::SetUseConsistentFloorMaterials(const bool UseConsistentFloorMaterials)
{
	bUseConsistentFloorMaterial = UseConsistentFloorMaterials;
	ForceCookPCG();
}

void APCG_BuildingGenerator::SetUseConsistentFloorHeight(const bool UseConsistentFloorHeight)
{
	bUseConsistentFloorHeight = UseConsistentFloorHeight;
	ForceCookPCG();
}

void APCG_BuildingGenerator::SetDesiredFloorHeight(const float NewFloorHeight)
{
    DesiredFloorHeight = NewFloorHeight;
    NumberOfFloors = FMath::Floor(BuildingHeight / DesiredFloorHeight);
    ForceCookPCG();
}

void APCG_BuildingGenerator::SetDesiredBuildingHeight(const float NewBuildingHeight)
{
    BuildingHeight = NewBuildingHeight;
    DesiredFloorHeight = BuildingHeight / NumberOfFloors;
    ForceCookPCG();
};



