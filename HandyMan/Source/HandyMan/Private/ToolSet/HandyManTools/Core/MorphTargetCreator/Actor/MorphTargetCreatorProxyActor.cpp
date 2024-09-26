// Fill out your copyright notice in the Description page of Project Settings.


#include "MorphTargetCreatorProxyActor.h"

#include "EditorAssetLibrary.h"
#include "Animation/SkeletalMeshActor.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshDecompositionFunctions.h"


// Sets default values
AMorphTargetCreatorProxyActor::AMorphTargetCreatorProxyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bEnableComputeMeshPool = false;
}

// Called when the game starts or when spawned
void AMorphTargetCreatorProxyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMorphTargetCreatorProxyActor::RebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{

	if(!bShouldStoreMeshIntoAsset) return;

	FGeometryScriptCopyMeshToAssetOptions CopyToMeshOptions;
	CopyToMeshOptions.bEnableRecomputeNormals = true;
	CopyToMeshOptions.bEnableRecomputeTangents = true;
	CopyToMeshOptions.bUseOriginalVertexOrder = true;
	EGeometryScriptOutcomePins CopyToMeshOutcome;

	FGeometryScriptMeshWriteLOD LodSettings;
	LodSettings.LODIndex = 0;
	auto CopiedMesh = UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToSkeletalMesh(TargetMesh, MorphTargetMesh, CopyToMeshOptions, LodSettings , CopyToMeshOutcome);

	/*if (CacheMeshData.bApplyChangesToAllLODS)
	{
		
	}
	else
	{
		for (const auto& LODIndex : CacheMeshData.LODSToWrite)
		{
			FGeometryScriptMeshWriteLOD LodSettings;
			LodSettings.LODIndex = LODIndex;
			auto CopiedMesh = UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToSkeletalMesh(TargetMesh, SavedSkeletalMesh, CopyToMeshOptions, LodSettings , CopyToMeshOutcome);
		}
	}*/
	
	ReleaseAllComputeMeshes();

	FActorSpawnParameters Params = FActorSpawnParameters();
	FString name = FString::Format(TEXT("Actor_{0}"), { MorphTargetMesh->GetFName().ToString() });
	FName fname = MakeUniqueObjectName(nullptr, ASkeletalMeshActor::StaticClass(), FName(*name));
	Params.Name = fname;
	Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
	// Spawn a skeletal mesh actor into the world with your asset
	auto SpawnedMesh = GetWorld()->SpawnActor<ASkeletalMeshActor>(GetActorLocation(), GetActorRotation(), Params);

	if (SpawnedMesh)
	{
		SpawnedMesh->GetSkeletalMeshComponent()->SetSkinnedAssetAndUpdate(MorphTargetMesh);
		MorphTargetMesh = nullptr;
		SetIsTemporarilyHiddenInEditor(true);
		SetLifeSpan(2.f);
	}
	
	Super::RebuildGeneratedMesh(TargetMesh);
}

// Called every frame
void AMorphTargetCreatorProxyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMorphTargetCreatorProxyActor::CacheBaseMesh(USkeletalMesh* InputMesh)
{
	if(!InputMesh) return;
	BaseMesh = AllocateComputeMesh();
	if(!BaseMesh) return;

	MorphTargetMesh = InputMesh;

	BaseMesh->Reset();

	FGeometryScriptCopyMeshFromAssetOptions CopyFromAssetOptions;
	CopyFromAssetOptions.bRequestTangents = true;
	CopyFromAssetOptions.bApplyBuildSettings = false;

	FGeometryScriptMeshReadLOD MeshRead;
	MeshRead.LODType = EGeometryScriptLODType::HiResSourceModel;

	EGeometryScriptOutcomePins CopyFromAssetOutcome;
	BaseMesh = UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromSkeletalMesh(InputMesh, BaseMesh, CopyFromAssetOptions, MeshRead, CopyFromAssetOutcome);

	if(!BaseMesh) return;

	UDynamicMesh* OutMesh;
	UGeometryScriptLibrary_MeshDecompositionFunctions::CopyMeshToMesh(BaseMesh, DynamicMeshComponent->GetDynamicMesh(), OutMesh);

	
}

void AMorphTargetCreatorProxyActor::RemoveMorphTargetMesh(const FName MorphTargetMeshName)
{
	if(MorphTargetMeshName.IsEqual(NAME_None)) return;

	if(!MorphTargetMeshMap.Contains(MorphTargetMeshName)) return;

	UDynamicMesh* GeneratedMesh = MorphTargetMeshMap[MorphTargetMeshName];

	if (GeneratedMesh == nullptr) return;

	ReleaseComputeMesh(GeneratedMesh);

	MorphTargetMeshMap.Remove(MorphTargetMeshName);
}

void AMorphTargetCreatorProxyActor::RemoveAllMorphTargetMeshes()
{
	for (const auto MeshMap : MorphTargetMeshMap)
	{
		RemoveMorphTargetMesh(MeshMap.Key);
	}
}

void AMorphTargetCreatorProxyActor::CreateMorphTargetMesh(FName MorphTargetMeshName)
{
	// Copy the base mesh into another allocated mesh and add it to the map. Set that mesh as the components mesh
	if(MorphTargetMeshName.IsEqual(NAME_None)) return;
	if(!MorphTargetMesh) return;
	
	UDynamicMesh* NewMorph = AllocateComputeMesh();
	
	if(!NewMorph) return;
	
	
	FGeometryScriptCopyMeshFromAssetOptions CopyFromAssetOptions;
	CopyFromAssetOptions.bRequestTangents = true;
	CopyFromAssetOptions.bApplyBuildSettings = false;

	FGeometryScriptMeshReadLOD MeshRead;
	MeshRead.LODType = EGeometryScriptLODType::HiResSourceModel;

	EGeometryScriptOutcomePins CopyFromAssetOutcome;
	NewMorph = UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromSkeletalMesh(MorphTargetMesh, NewMorph, CopyFromAssetOptions, MeshRead, CopyFromAssetOutcome);

	if(!NewMorph) return;

	UDynamicMesh* OutMesh;
	NewMorph = UGeometryScriptLibrary_MeshDecompositionFunctions::CopyMeshToMesh(NewMorph, DynamicMeshComponent->GetDynamicMesh(), OutMesh);

	MorphTargetMeshMap.Add(MorphTargetMeshName, NewMorph);
	
}

void AMorphTargetCreatorProxyActor::SaveObject()
{
	// Duplicate this asset in the same file path its in
	
	bShouldStoreMeshIntoAsset = true;

	RerunConstructionScripts();
}

