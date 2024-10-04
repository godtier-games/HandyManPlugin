// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletalMeshCutterActor.h"
#include "EditorAssetLibrary.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/BoxComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshDecompositionFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "Kismet/KismetMathLibrary.h"


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

	if(!LastSkeletalMesh) return;

	TargetMesh->Reset();

	auto ComputeMesh = AllocateComputeMesh();

	CombinedGeometrySelection = FGeometryScriptMeshSelection();

	FGeometryScriptCopyMeshFromAssetOptions CopyMeshOptions;
	CopyMeshOptions.bRequestTangents = true;
	FGeometryScriptMeshReadLOD LodReadSettings;
	EGeometryScriptOutcomePins CopyMeshOutcome;
	auto CopyMesh = UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromSkeletalMesh
	(LastSkeletalMesh, ComputeMesh, CopyMeshOptions, LodReadSettings, CopyMeshOutcome);

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
					if(!Shape->ComponentTags.Contains(FName(FString::Printf(TEXT("%d"), Cutter.Key)))) continue;

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
				if(!Shape->ComponentTags.Contains(FName(FString::Printf(TEXT("%d"), Cutter.Key)))) continue;

				FGeometryScriptMeshSelection Selection;
				UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsInSphere(CopyMesh, Selection, Shape->GetRelativeLocation(), Cast<USphereComponent>(Shape)->GetScaledSphereRadius());

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
	
	TargetMesh->SetMesh(*EditedMesh->ExtractMesh().Get());

	


	// Copy this mesh into
	if (bShouldStoreMeshIntoAsset && SavedSkeletalMesh)
	{
		FGeometryScriptCopyMeshToAssetOptions CopyToMeshOptions;
		CopyToMeshOptions.bEnableRecomputeNormals = true;
		CopyToMeshOptions.bEnableRecomputeTangents = true;
		CopyToMeshOptions.bUseOriginalVertexOrder = true;
		EGeometryScriptOutcomePins CopyToMeshOutcome;

		if (CacheMeshData.bApplyChangesToAllLODS)
		{
			for (int i = 0; i < SavedSkeletalMesh->GetLODNum(); ++i)
			{
				FGeometryScriptMeshWriteLOD LodSettings;
				LodSettings.LODIndex = i;
				auto CopiedMesh = UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToSkeletalMesh(TargetMesh, SavedSkeletalMesh, CopyToMeshOptions, LodSettings , CopyToMeshOutcome);
			}
		}
		else
		{
			for (const auto& LODIndex : CacheMeshData.LODSToWrite)
			{
				FGeometryScriptMeshWriteLOD LodSettings;
				LodSettings.LODIndex = LODIndex;
				auto CopiedMesh = UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToSkeletalMesh(TargetMesh, SavedSkeletalMesh, CopyToMeshOptions, LodSettings , CopyToMeshOutcome);
			}
		}
		ReleaseAllComputeMeshes();

		FActorSpawnParameters Params = FActorSpawnParameters();
		FString name = FString::Format(TEXT("Actor_{0}"), { SavedSkeletalMesh->GetFName().ToString() });
		FName fname = MakeUniqueObjectName(nullptr, ASkeletalMeshActor::StaticClass(), FName(*name));
		Params.Name = fname;
		Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
		// Spawn a skeletal mesh actor into the world with your asset
		auto SpawnedMesh = GetWorld()->SpawnActor<ASkeletalMeshActor>(GetActorLocation(), GetActorRotation(), Params);

		if (SpawnedMesh)
		{
			SpawnedMesh->GetSkeletalMeshComponent()->SetSkinnedAssetAndUpdate(SavedSkeletalMesh);
			LastSkeletalMesh = nullptr;
			SetIsTemporarilyHiddenInEditor(true);
			SetLifeSpan(2.f);
		}
	}
	else
	{
		ReleaseAllComputeMeshes();
		Super::RebuildGeneratedMesh(TargetMesh);
	}

	
}

FVector ASkeletalMeshCutterActor::AddNewCutter(const uint8& Index, const ECutterShapeType& Shape)
{
	Cutters.Add(Index, Shape);
	
	switch (Shape)
	{
	case ECutterShapeType::Box:
		{
			UBoxComponent* Box = NewObject<UBoxComponent>(this);
			Box->ComponentTags.Add(FName(*FString::Printf(TEXT("%d"), Index)));

			Box->RegisterComponent();
			AddInstanceComponent(Box);
			Box->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			Box->SetCollisionProfileName(FName("BlockAll"));
			Box->AddLocalOffset(FVector(0,0, 90 * Index));


			return Box->GetComponentLocation();
		}
	case ECutterShapeType::Sphere:
		{
			USphereComponent* Sphere = NewObject<USphereComponent>(this);
			Sphere->ComponentTags.Add(FName(*FString::Printf(TEXT("%d"), Index)));

			Sphere->RegisterComponent();
			AddInstanceComponent(Sphere);
			Sphere->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			Sphere->SetCollisionProfileName(FName("BlockAll"));
			Sphere->AddLocalOffset(FVector(0,0, 180));


			return Sphere->GetComponentLocation();
		}
	}


	return FVector::ZeroVector;
}

void ASkeletalMeshCutterActor::RemoveCutter(const uint8& Index)
{
	Cutters.Remove(Index);
}

void ASkeletalMeshCutterActor::RemoveCreatedAsset()
{
	if (SavedSkeletalMesh && SavedSkeletalMesh->IsAsset())
	{
		const FAssetData SavedAssetData = FAssetData(SavedSkeletalMesh, false);
		SavedSkeletalMesh = nullptr;
		const FString SavedAssetSourcePath = SavedAssetData.GetSoftObjectPath().ToString();
		UEditorAssetLibrary::DeleteAsset(SavedAssetSourcePath);
	}

	Destroy();
}



#if WITH_EDITOR
void ASkeletalMeshCutterActor::Initialize(const FSkeletalMeshAssetData& MeshData)
{
	if (!MeshData.InputMesh) return;

	// cache it as the last skeletal mesh
	LastSkeletalMesh = MeshData.InputMesh;
	CacheMeshData = MeshData;

	GetDynamicMeshComponent()->SetNumMaterials(MeshData.InputMesh->GetMaterials().Num());
	for (int i = 0; i < MeshData.InputMesh->GetMaterials().Num(); ++i)
	{
		const auto& Material = MeshData.InputMesh->GetMaterials()[i].MaterialInterface;
		GetDynamicMeshComponent()->SetMaterial(i, Material);
	}
	RerunConstructionScripts();
}

void ASkeletalMeshCutterActor::SaveObject()
{
	// Duplicate this asset in the same file path its in
	const auto AssetData = FAssetData(CacheMeshData.InputMesh, false);
		
	const FString AssetSourcePath = AssetData.GetSoftObjectPath().ToString();
	const FString NewSavePath = FString::Printf(TEXT("%s/%s/%s_%s"), *AssetData.PackagePath.ToString(), *CacheMeshData.FolderName, *AssetData.AssetName.ToString(), *CacheMeshData.SectionName);

	if (SavedSkeletalMesh && SavedSkeletalMesh->IsAsset())
	{
		const FAssetData SavedAssetData = FAssetData(SavedSkeletalMesh, false);
		
		const FString SavedAssetSourcePath = SavedAssetData.GetSoftObjectPath().ToString();
		UEditorAssetLibrary::DeleteAsset(SavedAssetSourcePath);
		SavedSkeletalMesh = nullptr;
	}
	
	if (USkeletalMesh* DuplicatedMesh = Cast<USkeletalMesh>(UEditorAssetLibrary::DuplicateAsset(AssetSourcePath, NewSavePath)))
	{
		SavedSkeletalMesh = DuplicatedMesh;
		/*for (int i = 0; i < SavedSkeletalMesh->GetLODNum(); ++i)
		{
			if(i == 0) continue;
			SavedSkeletalMesh->RemoveLODInfo(i);
		}*/
		/*SavedSkeletalMesh->SetMinLod(0);
		SavedSkeletalMesh->SetLODSettings(CutMeshSettings);*/

		
	
		UEditorAssetLibrary::SaveAsset(NewSavePath, false);

		SavedSkeletalMesh->GetOnMeshChanged().AddLambda([&]()
		{
			EditNum++;
			if (EditNum == SavedSkeletalMesh->GetLODNum())
			{
				UE_LOG(LogTemp, Warning, TEXT("Mesh Edits Complete"))
			}
		});
	}

	bShouldStoreMeshIntoAsset = true;

	RerunConstructionScripts();
	
}


#endif


