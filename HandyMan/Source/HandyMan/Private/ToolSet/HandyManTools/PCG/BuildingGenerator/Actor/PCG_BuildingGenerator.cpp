// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG_BuildingGenerator.h"

#include "PCGGraph.h"
#include "Engine/StaticMeshActor.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/MeshModelingFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "GeometryScript/MeshSelectionQueryFunctions.h"
#include "GeometryScript/MeshTransformFunctions.h"
#include "GeometryScript/PolyPathFunctions.h"
#include "GeometryScript/SceneUtilityFunctions.h"
#include "Helpers/PCGGraphParametersHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ModelingUtilities/GodtierModelingUtilities.h"
#include "PolyPathUtilities/GodtierPolyPathUtilities.h"
#include "SplineUtilities/GodtierSplineUtilities.h"


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

#if WITH_EDITOR
	if (OriginalMesh)
	{
		FActorSpawnParameters Params = FActorSpawnParameters();
		FString name = FString::Format(TEXT("DynamicActor_{0}"), { GetFName().ToString() });
		FName fname = MakeUniqueObjectName(nullptr, ADynamicMeshActor::StaticClass(), FName(*name));
		Params.Name = fname;
		Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
		
		if (auto NewDynamicActor = GetWorld()->SpawnActor<ADynamicMeshActor>(GetActorLocation(), GetActorRotation(), Params))
		{
			NewDynamicActor->GetDynamicMeshComponent()->SetDynamicMesh(OriginalMesh);
		}
	}
#endif
	
	
	Super::Destroyed();
}

void APCG_BuildingGenerator::BeginDestroy()
{
	Super::BeginDestroy();
}

void APCG_BuildingGenerator::RebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{
	ReleaseAllComputeMeshes();
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
	CreateBaseSplinesFromPolyPaths(PolyPaths);
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
	
	const auto Bounds = UGeometryScriptLibrary_MeshQueryFunctions::GetMeshBoundingBox(OriginalMesh);
	BuildingHeight = Bounds.GetExtent().Z * 2;

	InputActor->Destroy();
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

void APCG_BuildingGenerator::CreateBaseSplinesFromPolyPaths(const TArray<FGeometryScriptPolyPath>& Paths)
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
			if (BaseSplines.Contains(CurrentFloor))
			{
				BaseSplines[CurrentFloor]->SetSplinePoints(PathArray, ESplineCoordinateSpace::Local, true);
				ExpectedNumberOfSplines--;
				TempPaths.RemoveAtSwap(i);
				ZValues.RemoveAtSwap(IndexOfMinValue);
				CurrentFloor++;
			}
			else
			{
				USplineComponent* Spline = NewObject<USplineComponent>(this);
				Spline->SetSplinePoints(PathArray, ESplineCoordinateSpace::Local, true);
				FString TagString = CurrentFloor == 0 ? "Ground" : "Roof";
				Spline->ComponentTags.Add(FName(*TagString));
				Spline->RegisterComponent();
				AddInstanceComponent(Spline);
				Spline->AttachToComponent(GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));

				BaseSplines.Add(CurrentFloor, Spline);
				ExpectedNumberOfSplines--;
				TempPaths.RemoveAtSwap(i);
				ZValues.RemoveAtSwap(IndexOfMinValue);
				CurrentFloor++;
			}
		}
	}

	for (const auto Floor : BaseSplines)
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

void APCG_BuildingGenerator::CreateFloorSplinesFromPolyPaths(const TArray<FGeometryScriptPolyPath>& Paths)
{
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
				Spline->ComponentTags.Add(FName(*FString::Printf(TEXT("Floor_%d"), CurrentFloor + 1)));

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

}

void APCG_BuildingGenerator::RegenerateMesh()
{
	RerunConstructionScripts();
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
	
	CreateBaseSplinesFromPolyPaths(PolyPaths);
	ReleaseAllComputeMeshes();
}

void APCG_BuildingGenerator::BakeOpeningsToStatic()
{
	
}

void APCG_BuildingGenerator::CacheOpeningData(UBuildingGeneratorOpeningData* OpeningData)
{
	CachedOpeningData = OpeningData;
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

TArray<FGeometryScriptPolyPath> APCG_BuildingGenerator::UseTopFaceForFloor(UDynamicMesh* TargetMesh, const double FloorHeight, const bool bApplyAfterExtrude)
{
	// Duplicate the roof mesh
	auto* ComputeMesh = AllocateComputeMesh();
	auto FloorMesh = UGodtierModelingUtilities::GenerateMeshFromPlanarFace(ComputeMesh, OriginalMesh);

	// Move it down to ground level
	FloorMesh = UGeometryScriptLibrary_MeshTransformFunctions::TranslateMesh(FloorMesh, FVector(0, 0, -BuildingHeight));
		
		
	// Move it up to its floor level
	FloorMesh = UGeometryScriptLibrary_MeshTransformFunctions::TranslateMesh(FloorMesh, FVector(0, 0, (FloorHeight - WallThickness)));

	// Extrude the mesh to give it some thickness
	FGeometryScriptMeshSelection Selection;
	UGeometryScriptLibrary_MeshSelectionFunctions::CreateSelectAllMeshSelection(FloorMesh, Selection);

	TArray<FGeometryScriptPolyPath> ArrayOfPaths;
	if (!bApplyAfterExtrude)
	{
		ArrayOfPaths = UGodtierPolyPathUtilities::CreatePolyPathFromPlanarFaces(
		FloorMesh, {FVector(0, 0, 1)}, nullptr);
	
	}

	FGeometryScriptMeshLinearExtrudeOptions ExtrudeOptions;
	ExtrudeOptions.Direction = FVector(0, 0, 1);
	ExtrudeOptions.Distance = WallThickness;
	ExtrudeOptions.AreaMode = EGeometryScriptPolyOperationArea::EntireSelection;
	
	FloorMesh = UGeometryScriptLibrary_MeshModelingFunctions::ApplyMeshLinearExtrudeFaces(FloorMesh, ExtrudeOptions, Selection);

	if (bApplyAfterExtrude)
	{
		ArrayOfPaths = UGodtierPolyPathUtilities::CreatePolyPathFromPlanarFaces(
		FloorMesh, {FVector(0, 0, 1)}, nullptr);
	}

	// Append it to the existing mesh.
	UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(TargetMesh, FloorMesh, FTransform::Identity);

	return ArrayOfPaths;
}

void APCG_BuildingGenerator::GenerateFloorMeshes(UDynamicMesh* TargetMesh)
{
	if (!TargetMesh)
	{
		return;
	}

	/*
	if (NumberOfFloors > 2)
	{
		RecalculateFloorClearance();
	}
	*/
	
	TArray<FGeometryScriptPolyPath> NewFloorPaths;
	
	for (int i = 0; i < NumberOfFloors; ++i)
	{
		// Skip the base floor this will be generated either from the underlying geometry or from PCG - TODO : Make this an option?
		if(i == 0) continue;
		
		double FloorHeight = i == 1 ? BaseFloorHeight : BaseFloorHeight + ((i - 1) * DesiredFloorClearance);

		if (FloorHeight > BuildingHeight)
		{
			continue;
		}
		
		NewFloorPaths.Append(UseTopFaceForFloor(TargetMesh, FloorHeight, true));
		
	}


	if (bGenerateSplinesForEachFloor)
	{
		CreateFloorSplinesFromPolyPaths(NewFloorPaths);
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
	
	if (BaseSplines.Contains(0))
	{
		FSimpleCollisionOptions SweepOptions;
		SweepOptions.TargetMesh = TempMesh;
		SweepOptions.Spline = BaseSplines[0];
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
	auto* Booleans = AllocateComputeMesh();
	for (const auto& Entry : GeneratedOpenings)
	{
		float CurrentSizeX = 1.f;

		FVector Extent = FVector::One();

		if (Entry.Key.IsA(UStaticMesh::StaticClass()))
		{
			Extent = Cast<UStaticMesh>(Entry.Key)->GetBoundingBox().GetExtent();
			CurrentSizeX = Cast<UStaticMesh>(Entry.Key)->GetBoundingBox().GetExtent().X;
		}

		// TODO : Handle Actors being used as openings

		
		for (const auto Opening : Entry.Value.Openings)
		{
			if(!Opening.bShouldApplyBoolean) continue;

			if(!Opening.Mesh) continue;

			auto* ComputeMesh = AllocateComputeMesh();
			ComputeMesh->Reset();

			/*float CurrentSizeX = Opening.Mesh->GetActorRelativeScale3D().X;

			FVector Origin;
			FVector Extent;
			Opening.Mesh->GetActorBounds(false, Origin, Extent);

			CurrentSizeX = Extent.X;*/

			float ScaleOffset = .35f;
			if (WallThickness > CurrentSizeX)
			{
				ScaleOffset = FMath::Max((WallThickness/CurrentSizeX), 5.f);
			}

			const float Offset = Opening.bShouldCutHoleInTargetMesh ? ScaleOffset : 0.f;
		
			ComputeMesh = UGodtierModelingUtilities::CreateDynamicBooleanMesh(ComputeMesh, Opening.Mesh, Opening.BooleanShape, Offset , nullptr);

			const auto RelativeTransform = UKismetMathLibrary::MakeRelativeTransform(Opening.Mesh->GetActorTransform(), GetActorTransform());
			UGeometryScriptLibrary_MeshTransformFunctions::TransformMesh(ComputeMesh, RelativeTransform, false);

			UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(Booleans, ComputeMesh, FTransform::Identity);
			// Cut this mesh from the target mesh
			FGeometryScriptMeshBooleanOptions BooleanOptions;
			BooleanOptions.bFillHoles = false;
		
			UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshBoolean
			(
				TargetMesh,
				FTransform::Identity,
				ComputeMesh,
				FTransform::Identity,
				Opening.bShouldCutHoleInTargetMesh ? EGeometryScriptBooleanOperation::Subtract : EGeometryScriptBooleanOperation::Union,
				BooleanOptions
			);

			if (!Opening.bShouldCutHoleInTargetMesh && Opening.bShouldApplyBoolean)
			{
				// Hide the actor in the world outliner
				Opening.Mesh->SetIsTemporarilyHiddenInEditor(true);
			}

			if (bMaintainOpeningProportions)
			{
				FVector MeshScale = FVector::OneVector;
				switch (Opening.Fit)
				{
				case EMeshFitStyle::Flush:
					{
						const float CurrentX = (Extent.X* 2);
						if (WallThickness > CurrentX)
						{
							MeshScale = FVector(WallThickness / CurrentX, 1.0f, 1.0f);
						}
					}
					break;
				case EMeshFitStyle::In_Front:
					break;
				case EMeshFitStyle::Centered:
					{
						const float CurrentX = (Extent.X* 2);
						if (!FMath::IsNearlyEqual(WallThickness, CurrentX))
						{
							const auto XScale = (WallThickness / CurrentX) + (Opening.CenteredOffset * .01f);
							MeshScale = FVector(XScale, Opening.Mesh->GetRootComponent()->GetRelativeScale3D().Y, Opening.Mesh->GetRootComponent()->GetRelativeScale3D().Z);
						}
						else
						{
							MeshScale = FVector(1.0f + (Opening.CenteredOffset * .01f), Opening.Mesh->GetRootComponent()->GetRelativeScale3D().Y, Opening.Mesh->GetRootComponent()->GetRelativeScale3D().Z);
						}
					}
					break;
				}

				Opening.Mesh->GetRootComponent()->SetRelativeScale3D(MeshScale);
			}

			/*bool bDebugBooleanMeshes = true;

			if (bDebugBooleanMeshes)
			{
				UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(TargetMesh, Booleans, UKismetMathLibrary::MakeRelativeTransform(Opening.Mesh->GetActorTransform(), GetActorTransform()));
			}*/
		}
	}

	
}

#if WITH_EDITOR
void APCG_BuildingGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, NumberOfFloors))
	{
		SetNumberOfFloors(NumberOfFloors);
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, BaseFloorHeight))
	{
		SetBaseFloorHeight(BaseFloorHeight);
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, BuildingHeight))
	{
		SetDesiredBuildingHeight(BuildingHeight);
	}
	
	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, DesiredFloorClearance))
	{
		SetDesiredFloorClearance(DesiredFloorClearance);
	}
}
	
#endif


void APCG_BuildingGenerator::ForceCookPCG()
{
	PCG->CleanupLocalImmediate(true, true);
	PCG->Generate(true);
	PCG->NotifyPropertiesChangedFromBlueprint();

}

void APCG_BuildingGenerator::RecalculateFloorClearance()
{
	const float Delta = BuildingHeight - BaseFloorHeight;

	const int32 Offset = NumberOfFloors >= 3 ? 1 : 0;
	SetDesiredFloorClearance(Delta/(NumberOfFloors - Offset));
}

void APCG_BuildingGenerator::SetNumberOfFloors(const int32 NewFloorCount)
{
	NumberOfFloors = NewFloorCount;

	if (bUseConsistentFloorHeight)
	{
		RecalculateFloorClearance();
		return;
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

void APCG_BuildingGenerator::SetBaseFloorHeight(const float NewFloorHeight)
{
    BaseFloorHeight = NewFloorHeight;

	if (PCG && PCG->GetGraphInstance())
	{
		UPCGGraphParametersHelpers::SetFloatParameter(PCG->GetGraphInstance(), "MinFloorClearance", WallThickness);
	}

    if (bUseConsistentFloorHeight)
    {
	   RecalculateFloorClearance();
    	return;
    }
	RerunConstructionScripts();

}

void APCG_BuildingGenerator::SetDesiredFloorClearance(const float NewClearance)
{
	DesiredFloorClearance = NewClearance;
	
	if (PCG && PCG->GetGraphInstance())
	{
		UPCGGraphParametersHelpers::SetFloatParameter(PCG->GetGraphInstance(), "MinFloorClearance", WallThickness);
	}
	
	RerunConstructionScripts();
}

void APCG_BuildingGenerator::SetDesiredBuildingHeight(const float NewBuildingHeight)
{
	const auto Bounds = UGeometryScriptLibrary_MeshQueryFunctions::GetMeshBoundingBox(OriginalMesh);
	const float LastHeight = Bounds.GetExtent().Z * 2;
	
    BuildingHeight = NewBuildingHeight;
	float multiplier = LastHeight > BuildingHeight ? -1 : 1;
	const float Delta = FMath::Abs(NewBuildingHeight - LastHeight);

	// Update the Input mesh since we use it to generate meshes repeatedly.
	FGeometryScriptMeshSelection Selection;
	UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsByNormalAngle(OriginalMesh, Selection);
	OriginalMesh = UGeometryScriptLibrary_MeshTransformFunctions::TranslateMeshSelection(OriginalMesh, Selection, FVector(0, 0, Delta * multiplier));
	
	CreateFloorAndRoofSplines();
	
	RerunConstructionScripts();

}

void APCG_BuildingGenerator::SetWallThickness(const float Thickness)
{
	WallThickness = Thickness;

	if (PCG && PCG->GetGraphInstance())
	{
		UPCGGraphParametersHelpers::SetFloatParameter(PCG->GetGraphInstance(), "WallThickness", WallThickness);
	}
	
	RerunConstructionScripts();
};


