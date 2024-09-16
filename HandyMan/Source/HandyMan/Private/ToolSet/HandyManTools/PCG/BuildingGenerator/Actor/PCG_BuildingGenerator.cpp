// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG_BuildingGenerator.h"
#include "Engine/StaticMeshActor.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/MeshModelingFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "GeometryScript/MeshTransformFunctions.h"
#include "GeometryScript/PolyPathFunctions.h"
#include "GeometryScript/SceneUtilityFunctions.h"
#include "ModelingUtilities/GodtierModelingUtilities.h"
#include "PolyPathUtilities/GodtierPolyPathUtilities.h"


// Sets default values
APCG_BuildingGenerator::APCG_BuildingGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PCG = CreateDefaultSubobject<UPCGComponent>(TEXT("PCG"));

	DynamicMeshComponent->CollisionType = CTF_UseComplexAsSimple;
	DynamicMeshComponent->bEnableComplexCollision = true;
	
}

// Called when the game starts or when spawned
void APCG_BuildingGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

void APCG_BuildingGenerator::Destroyed()
{
	for (const auto Entry : GeneratedOpenings)
	{
		for (const auto Opening : Entry.Value.Openings)
		{
			if (!Opening.Mesh) continue;
			
			Opening.Mesh->Destroy();
		}
	}

	if (OriginalActor)
	{
		OriginalActor->SetIsTemporarilyHiddenInEditor(false);
	}
	
	Super::Destroyed();
}

void APCG_BuildingGenerator::RebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{
	GenerateExteriorWalls(TargetMesh);
	GenerateFloorMeshes(TargetMesh);
	GenerateRoofMesh(TargetMesh);

	ForceCookPCG();
	
	Super::RebuildGeneratedMesh(TargetMesh);
	
	
}

void APCG_BuildingGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void APCG_BuildingGenerator::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void APCG_BuildingGenerator::CreateFloorAndRoofSplines()
{
	TArray<FVector> NormalDirections;
	NormalDirections.Add(FVector(0, 0, 1));
	NormalDirections.Add(FVector(0, 0, -1));
	
	auto PolyPaths = UGodtierPolyPathUtilities::CreatePolyPathFromPlanarFaces(OriginalMesh, NormalDirections, nullptr);
	CreateSplinesFromPolyPaths(PolyPaths);
}

void APCG_BuildingGenerator::CacheInputActor(AActor* InputActor)
{
	OriginalMesh = NewObject<UDynamicMesh>(this);
	EGeometryScriptOutcomePins Outcome;
	FGeometryScriptCopyMeshFromComponentOptions Options;
	FTransform LocalToWorld;
	
	OriginalMesh = UGeometryScriptLibrary_SceneUtilityFunctions::CopyMeshFromComponent
	(
		InputActor->GetRootComponent(),
		OriginalMesh,
		Options,
		false,
		LocalToWorld,
		Outcome
	);
	
	OriginalActor = InputActor;
	FVector Origin;
	FVector Extent;
	InputActor->GetActorBounds(false, Origin, Extent);
	BuildingHeight = Extent.Z * 2;

	DesiredFloorHeight = BuildingHeight / NumberOfFloors;

	InputActor->SetIsTemporarilyHiddenInEditor(true);
	CreateFloorAndRoofSplines();
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
		
			int32 IndexOfMinValue;
			const float MinValue = FMath::Min(ZValues, &IndexOfMinValue);

			if (!FMath::IsNearlyEqual(Path.Path->GetData()[0].Z, MinValue)) continue;

			TArray<FVector> PathArray;
			UGeometryScriptLibrary_PolyPathFunctions::ConvertPolyPathToArray(Path, PathArray);

			// Create a spline component and add it to the map. If a spline component already exists for that floor just set its points to the new path.
			if (FloorSplines.Contains(CurrentFloor))
			{
				FloorSplines[CurrentFloor]->SetSplinePoints(PathArray, ESplineCoordinateSpace::Local, true);
				ExpectedNumberOfSplines--;
				TempPaths.RemoveAtSwap(i);
				ZValues.RemoveAtSwap(IndexOfMinValue);
				CurrentFloor++;
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
				ZValues.RemoveAtSwap(IndexOfMinValue);
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

	GenerateFloorMeshes(GetDynamicMeshComponent()->GetDynamicMesh());

	GenerateRoofMesh(GetDynamicMeshComponent()->GetDynamicMesh());
	
	
}

void APCG_BuildingGenerator::GenerateSplinesFromGeneratedMesh()
{
	TArray<FVector> NormalDirections;
	NormalDirections.Add(FVector(0, 0, 1));
	NormalDirections.Add(FVector(0, 0, -1));

	auto TempMesh = AllocateComputeMesh();
	TempMesh->Reset();
	TempMesh->SetMesh(*GetDynamicMeshComponent()->GetMesh());

	// Merge this mesh so that faces have a singular border.
	FGeometryScriptMeshSelfUnionOptions SelfUnionOptions;
	SelfUnionOptions.bFillHoles = false;
	TempMesh = UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshSelfUnion(TempMesh, SelfUnionOptions);
	
	auto PolyPaths = UGodtierPolyPathUtilities::CreatePolyPathFromPlanarFaces(TempMesh, NormalDirections, nullptr);
	
	CreateSplinesFromPolyPaths(PolyPaths);
	ReleaseAllComputeMeshes();
}

void APCG_BuildingGenerator::BakeOpeningsToStatic()
{
	
}

void APCG_BuildingGenerator::AddGeneratedOpeningEntry(const FGeneratedOpening& Entry)
{
	if (Entry.Mesh.IsA(AStaticMeshActor::StaticClass()))
	{
		auto Key = Cast<AStaticMeshActor>(Entry.Mesh)->GetStaticMeshComponent()->GetStaticMesh();
		if(!GeneratedOpenings.Contains(Key))
		{
			GeneratedOpenings.Add(Key, FGeneratedOpeningArray(Entry));
		}
		else
		{
			GeneratedOpenings[Key].Openings.Add(Entry);
		}
			
	}
	else
	{
		if (!GeneratedOpenings.Contains(Entry.Mesh))
		{
			GeneratedOpenings.Add(Entry.Mesh, FGeneratedOpeningArray(Entry));
		}
		else
		{
			GeneratedOpenings[Entry.Mesh].Openings.Add(Entry);
		}
	}
		
	
	RerunConstructionScripts();
}

void APCG_BuildingGenerator::RemoveGeneratedOpeningEntry(const FGeneratedOpening& Entry)
{
	if (GeneratedOpenings.Contains(Entry.Mesh))
	{
		GeneratedOpenings[Entry.Mesh].Openings.RemoveSingle(Entry);
	}
	else if (Entry.Mesh.IsA(AStaticMeshActor::StaticClass()))
	{
		auto Key = Cast<AStaticMeshActor>(Entry.Mesh)->GetStaticMeshComponent()->GetStaticMesh();
		if(!GeneratedOpenings.Contains(Key)) return;
		GeneratedOpenings[Key].Openings.RemoveSingle(Entry);
	}
	
	RerunConstructionScripts();
}

void APCG_BuildingGenerator::UpdatedGeneratedOpenings(const TArray<FGeneratedOpening>& Entries)
{
	GeneratedOpenings.Empty();
	for (const auto Entry : Entries)
	{
		if (Entry.Mesh.IsA(AStaticMeshActor::StaticClass()))
		{
			auto Key = Cast<AStaticMeshActor>(Entry.Mesh)->GetStaticMeshComponent()->GetStaticMesh();
			if(!GeneratedOpenings.Contains(Key))
			{
				GeneratedOpenings.Add(Key, FGeneratedOpeningArray(Entry));
			}
			else
			{
				GeneratedOpenings[Key].Openings.Add(Entry);
			}
			
		}
		else
		{
			if (!GeneratedOpenings.Contains(Entry.Mesh))
			{
				GeneratedOpenings.Add(Entry.Mesh, FGeneratedOpeningArray(Entry));
			}
			else
			{
				GeneratedOpenings[Entry.Mesh].Openings.Add(Entry);
			}
		}
		
	}
	RerunConstructionScripts();
}

TArray<FGeneratedOpening> APCG_BuildingGenerator::GetGeneratedOpenings(const UObject* Key) const
{
	if (GeneratedOpenings.Contains(Key))
	{
		return GeneratedOpenings[Key].Openings;
	}

	return TArray<FGeneratedOpening>();
}

TArray<FGeneratedOpening> APCG_BuildingGenerator::GetGeneratedOpenings() const
{
	TArray<FGeneratedOpening> OutArray;

	for (const auto Entry : GeneratedOpenings)
	{
		for (const auto Opening : Entry.Value.Openings)
		{
			OutArray.Add(Opening);
		}
	}

	return OutArray;
}

TMap<TObjectPtr<UObject>, FGeneratedOpeningArray> APCG_BuildingGenerator::GetGeneratedOpeningsMap() const
{
	return GeneratedOpenings;
}

void APCG_BuildingGenerator::GenerateRoofMesh(UDynamicMesh* TargetMesh)
{
	if (!TargetMesh)
	{
		return;
	}

	// TODO - Here I would like to procedurally place smart meshes along the roof spline.
}

void APCG_BuildingGenerator::UseTopFaceForFloor(UDynamicMesh* TargetMesh, const double FloorHeight)
{
	// Duplicate the roof mesh
	auto* ComputeMesh = AllocateComputeMesh();
	auto RoofMesh = UGodtierModelingUtilities::GenerateMeshFromPlanarFace(ComputeMesh, OriginalActor);

	// Move it down to ground level
	RoofMesh = UGeometryScriptLibrary_MeshTransformFunctions::TranslateMesh(RoofMesh, FVector(0, 0, -BuildingHeight));
		
		
	// Move it up to its floor level
	RoofMesh = UGeometryScriptLibrary_MeshTransformFunctions::TranslateMesh(RoofMesh, FVector(0, 0, (FloorHeight - WallThickness)));

	// Extrude the mesh to give it some thickness
	FGeometryScriptMeshSelection Selection;
	UGeometryScriptLibrary_MeshSelectionFunctions::CreateSelectAllMeshSelection(RoofMesh, Selection);

	FGeometryScriptMeshLinearExtrudeOptions ExtrudeOptions;
	ExtrudeOptions.Direction = FVector(0, 0, 1);
	ExtrudeOptions.Distance = WallThickness;
	ExtrudeOptions.AreaMode = EGeometryScriptPolyOperationArea::EntireSelection;
	
	RoofMesh = UGeometryScriptLibrary_MeshModelingFunctions::ApplyMeshLinearExtrudeFaces(RoofMesh, ExtrudeOptions, Selection);

	// Append it to the existing mesh.
	UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(TargetMesh, RoofMesh, FTransform::Identity);
	
	ReleaseAllComputeMeshes();
}

void APCG_BuildingGenerator::GenerateFloorMeshes(UDynamicMesh* TargetMesh)
{
	if (!TargetMesh)
	{
		return;
	}


	for (int i = 0; i < NumberOfFloors; ++i)
	{
		// Skip the base floor this will be generated either from the underlying geometry or from PCG - TODO : Make this an option?
		double FloorHeight = 0;
		if (bUseConsistentFloorHeight)
		{
			FloorHeight = i * DesiredFloorHeight;
		}
		else
		{
			if (DesiredFloorHeightMap.Contains(i))
			{
				FloorHeight = DesiredFloorHeightMap[i];
			}
		}
		
		if(i == 0) continue;


		if (bGenerateSplinesForEachFloor)
		{
			if (FloorSplines.Contains(i))
			{
				// Move the spline to the correct height
			}
			else
			{
				// Generate a new spline from the base spline and move it up to the floor height.
			}
		}
		
		UseTopFaceForFloor(TargetMesh, FloorHeight);
		
	}
	
	
	// If the building has an open roof do not create the mesh for it.
	if (!bHasOpenRoof)
	{
		UseTopFaceForFloor(TargetMesh, BuildingHeight);
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

	AppendOpeningToMesh(CombinedSplinesMesh);

	UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(TargetMesh ? TargetMesh : GetDynamicMeshComponent()->GetDynamicMesh(), CombinedSplinesMesh, FTransform::Identity);
	if (!BuildingMaterial.ToString().IsEmpty())
	{
		DynamicMeshComponent->SetMaterial(0, BuildingMaterial.LoadSynchronous());
	}

	
	
}

void APCG_BuildingGenerator::AppendOpeningToMesh(UDynamicMesh* TargetMesh)
{
	// Iterate over the generated openings
	for (const auto& Entry : GeneratedOpenings)
	{
		for (const auto Opening : Entry.Value.Openings)
		{
			if(!Opening.bShouldApplyBoolean) continue;

			auto* ComputeMesh = AllocateComputeMesh();


			const float Offset = Opening.bShouldCutHoleInTargetMesh ? .35f : 0.f;
		
			auto* BoolMesh = UGodtierModelingUtilities::CreateDynamicBooleanMesh(ComputeMesh, Opening.Mesh, Opening.BooleanShape, Offset , nullptr);

			// Cut this mesh from the target mesh
			FGeometryScriptMeshBooleanOptions BooleanOptions;
			BooleanOptions.bFillHoles = false;
		
			UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshBoolean
			(
				TargetMesh,
				GetActorTransform(),
				BoolMesh,
				Opening.Mesh->GetActorTransform(),
				Opening.bShouldCutHoleInTargetMesh ? EGeometryScriptBooleanOperation::Subtract : EGeometryScriptBooleanOperation::Union,
				BooleanOptions
			);

			if (!Opening.bShouldCutHoleInTargetMesh && Opening.bShouldApplyBoolean)
			{
				// Hide the actor in the world outliner
				Opening.Mesh->SetIsTemporarilyHiddenInEditor(true);
			}
		}
	}
	// For each opening get its base shape and create a dynamic mesh from it.
	// Use the dynamic mesh to cut a hole in the target mesh.
}

#if WITH_EDITOR
void APCG_BuildingGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, NumberOfFloors))
	{
		SetNumberOfFloors(NumberOfFloors);
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, DesiredFloorHeight))
	{
		SetDesiredFloorHeight(DesiredFloorHeight);
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, BuildingHeight))
	{
		SetDesiredBuildingHeight(BuildingHeight);
	}
	
}
#endif


void APCG_BuildingGenerator::ForceCookPCG()
{
	PCG->CleanupLocalImmediate(true, true);
	PCG->NotifyPropertiesChangedFromBlueprint();
}

void APCG_BuildingGenerator::SetNumberOfFloors(const int32 NewFloorCount)
{
	NumberOfFloors = NewFloorCount;
	if (bUseConsistentFloorHeight)
	{
		DesiredFloorHeight = BuildingHeight / NumberOfFloors;
	}
	else
	{
		
	}
	
	RerunConstructionScripts();
}

void APCG_BuildingGenerator::SetHasOpenRoof(const bool HasOpenRoof)
{
    bHasOpenRoof = HasOpenRoof;
	RerunConstructionScripts();

}

void APCG_BuildingGenerator::SetUseConsistentFloorMaterials(const bool UseConsistentFloorMaterials)
{
	bUseConsistentFloorMaterial = UseConsistentFloorMaterials;
	RerunConstructionScripts();

	
}

void APCG_BuildingGenerator::SetUseConsistentFloorHeight(const bool UseConsistentFloorHeight)
{
	bUseConsistentFloorHeight = UseConsistentFloorHeight;
	RerunConstructionScripts();

	
}

void APCG_BuildingGenerator::SetDesiredFloorHeight(const float NewFloorHeight)
{
    DesiredFloorHeight = NewFloorHeight;
    NumberOfFloors = FMath::Floor(BuildingHeight / DesiredFloorHeight);
	RerunConstructionScripts();

}

void APCG_BuildingGenerator::SetDesiredBuildingHeight(const float NewBuildingHeight)
{
	FVector Origin;
	FVector Extent;
	OriginalActor->GetActorBounds(false, Origin, Extent);
	const float LastHeight = Extent.Z * 2;
	
    BuildingHeight = NewBuildingHeight;
    DesiredFloorHeight = BuildingHeight / NumberOfFloors;

	float multiplier = LastHeight > BuildingHeight ? -1 : 1;
	const float Delta = FMath::Abs(NewBuildingHeight - LastHeight);

	// Update the Input mesh since we use it to generate meshes repeatedly.
	FGeometryScriptMeshSelection Selection;
	UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsByNormalAngle(OriginalMesh, Selection);
	OriginalMesh = UGeometryScriptLibrary_MeshTransformFunctions::TranslateMeshSelection(OriginalMesh, Selection, FVector(0, 0, Delta * multiplier));
	
	CreateFloorAndRoofSplines();
	RerunConstructionScripts();

};



