// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletalMeshCutterActor.h"

#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryHelpers.h"
#include "Components/BoxComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
ASkeletalMeshCutterActor::ASkeletalMeshCutterActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASkeletalMeshCutterActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASkeletalMeshCutterActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASkeletalMeshCutterActor::RebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{

	if(!SavedSkeletalMesh || !LastSkeletalMesh) return;

	TargetMesh->Reset();

	auto ComputeMesh = AllocateComputeMesh();

	CombinedGeometrySelection = FGeometryScriptMeshSelection();

	FGeometryScriptCopyMeshFromAssetOptions CopyMeshOptions;
	EGeometryScriptOutcomePins CopyMeshOutcome;
	auto CopyMesh = UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromSkeletalMesh
	(LastSkeletalMesh, ComputeMesh, CopyMeshOptions, FGeometryScriptMeshReadLOD(), CopyMeshOutcome);

	// make an array of selections and keep combining them with the cached selection.

	TArray<UShapeComponent*> CutterShapes;
	GetComponents(UShapeComponent::StaticClass(), CutterShapes);
	for (const auto& Cutter : Cutters)
	{
		switch (Cutter.Value)
		{
		case ECutterShapeType::Box:
			{
				for (const auto& Shape : CutterShapes)
				{
					if(!Shape->IsA(UBoxComponent::StaticClass())) continue;
					if(!Shape->ComponentTags.Contains(FName(FString::Printf(TEXT("Box_%d"), Cutter.Key)))) continue;

					const auto Box = UKismetMathLibrary::MakeBoxWithOrigin(Shape->GetRelativeLocation(), Cast<UBoxComponent>(Shape)->GetScaledBoxExtent());
					FGeometryScriptMeshSelection Selection;
					UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsInBox(CopyMesh, Selection, Box);

					FGeometryScriptMeshSelection CombinedSelection;
					UGeometryScriptLibrary_MeshSelectionFunctions::CombineMeshSelections(Selection, CombinedGeometrySelection, CombinedSelection);
					CombinedGeometrySelection = CombinedSelection;
					break;
				}
			}
			
			break;
		case ECutterShapeType::Sphere:

			for (const auto& Shape : CutterShapes)
			{
				if(!Shape->IsA(USphereComponent::StaticClass())) continue;
				if(!Shape->ComponentTags.Contains(FName(FString::Printf(TEXT("Sphere_%d"), Cutter.Key)))) continue;

				FGeometryScriptMeshSelection Selection;
				UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsInSphere(CopyMesh, Selection, Shape->GetRelativeLocation(),Cast<USphereComponent>(Shape)->GetScaledSphereRadius());

				FGeometryScriptMeshSelection CombinedSelection;
				UGeometryScriptLibrary_MeshSelectionFunctions::CombineMeshSelections(Selection, CombinedGeometrySelection, CombinedSelection);
				CombinedGeometrySelection = CombinedSelection;
				break;
			}
			break;
		}
	}

	// Cut the mesh using the combined selection
	int32 NumDeleted;
	auto EditedMesh = UGeometryScriptLibrary_MeshBasicEditFunctions::DeleteSelectedTrianglesFromMesh(CopyMesh, CombinedGeometrySelection, NumDeleted);

	// Copy this mesh into
	FGeometryScriptCopyMeshToAssetOptions CopyToMeshOptions;
	EGeometryScriptOutcomePins CopyToMeshOutcome;
	auto CopiedMesh = UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToSkeletalMesh(EditedMesh, SavedSkeletalMesh, CopyToMeshOptions, FGeometryScriptMeshWriteLOD(), CopyToMeshOutcome);

	TargetMesh->SetMesh(*CopiedMesh->ExtractMesh().Get());

	ReleaseAllComputeMeshes();
	Super::RebuildGeneratedMesh(TargetMesh);

	
}

FVector ASkeletalMeshCutterActor::AddNewCutter(const uint8& Index, const ECutterShapeType& Shape)
{
	Cutters.Add(Index, Shape);
	
	switch (Shape)
	{
	case ECutterShapeType::Box:
		{
			UBoxComponent* Box = NewObject<UBoxComponent>(this);
			Box->ComponentTags.Add(FName(*FString::Printf(TEXT("Box_%d"), Index)));

			Box->RegisterComponent();
			AddInstanceComponent(Box);
			Box->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

			return Box->GetComponentLocation();
		}
	case ECutterShapeType::Sphere:
		{
			USphereComponent* Sphere = NewObject<USphereComponent>(this);
			Sphere->ComponentTags.Add(FName(*FString::Printf(TEXT("Sphere_%d"), Index)));

			Sphere->RegisterComponent();
			AddInstanceComponent(Sphere);
			Sphere->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

			return Sphere->GetComponentLocation();
		}
	}


	return FVector::ZeroVector;
}

void ASkeletalMeshCutterActor::RemoveCutter(const uint8& Index)
{
	Cutters.Remove(Index);
}

#if WITH_EDITOR

  void ASkeletalMeshCutterActor::Initialize(const FSkeletalMeshAssetData& MeshData)
{
	if (!MeshData.InputMesh) return;

	// cache it as the last skeletal mesh
	LastSkeletalMesh = MeshData.InputMesh;
	// Duplicate this asset in the same file path its in
	const auto AssetData = UAssetRegistryHelpers::CreateAssetData(MeshData.InputMesh);
		
	const FString AssetSourcePath = AssetData.GetSoftObjectPath().ToString();
	const FString NewSavePath = FString::Printf(TEXT("%s/%s_%s"), *AssetData.PackagePath.ToString(), *AssetData.AssetName.ToString(), *MeshData.SectionName);

	if (SavedSkeletalMesh)
	{
		const auto SavedAssetData = UAssetRegistryHelpers::CreateAssetData(SavedSkeletalMesh);
		
		const FString SavedAssetSourcePath = SavedAssetData.GetSoftObjectPath().ToString();
		UEditorAssetLibrary::DeleteAsset(SavedAssetSourcePath);
		SavedSkeletalMesh = nullptr;
	}
	
	if (USkeletalMesh* DuplicatedMesh = Cast<USkeletalMesh>(UEditorAssetLibrary::DuplicateAsset(AssetSourcePath, NewSavePath)))
	{
		SavedSkeletalMesh = DuplicatedMesh;
	}
	
	RerunConstructionScripts();
}


#endif


