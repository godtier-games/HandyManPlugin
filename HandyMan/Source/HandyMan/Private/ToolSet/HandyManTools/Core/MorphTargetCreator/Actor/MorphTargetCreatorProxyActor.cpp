// Fill out your copyright notice in the Description page of Project Settings.


#include "MorphTargetCreatorProxyActor.h"

#include "EditorAssetLibrary.h"
#include "UDynamicMesh.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/DynamicMeshComponent.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshDecompositionFunctions.h"


// Sets default values
AMorphTargetCreatorProxyActor::AMorphTargetCreatorProxyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMorphTargetCreatorProxyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMorphTargetCreatorProxyActor::Destroyed()
{
	Super::Destroyed();
}

// Called every frame
void AMorphTargetCreatorProxyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMorphTargetCreatorProxyActor::CacheBaseMesh(USkeletalMesh* InputMesh)
{
	if(!InputMesh) return;
	BaseMesh = NewObject<UDynamicMesh>(this);
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
	UGeometryScriptLibrary_MeshDecompositionFunctions::CopyMeshToMesh(BaseMesh, FindComponentByClass<UDynamicMeshComponent>()->GetDynamicMesh(), OutMesh);
}

void AMorphTargetCreatorProxyActor::RemoveMorphTargetMesh(const FName& MorphTargetMeshName, const bool bShouldRestoreMesh)
{
	if(MorphTargetMeshName.IsEqual(NAME_None)) return;

	if(!MorphTargetMeshMap.Contains(MorphTargetMeshName)) return;

	MorphTargetMeshMap.Remove(MorphTargetMeshName);

	if (bShouldRestoreMesh)
	{
		RestoreLastMorphTarget();
	}
}

void AMorphTargetCreatorProxyActor::RemoveAllMorphTargetMeshes()
{
	for (const auto MeshMap : MorphTargetMeshMap)
	{
		RemoveMorphTargetMesh(MeshMap.Key);
	}
}

void AMorphTargetCreatorProxyActor::CreateMorphTargetMesh(const FName& MorphTargetMeshName)
{
	// Copy the base mesh into another allocated mesh and add it to the map. Set that mesh as the components mesh
	if(MorphTargetMeshName.IsEqual(NAME_None)) return;
	if(!MorphTargetMesh) return;

	// Copy the last morph mesh into the object we previously created
	if (MorphTargetMeshMap.Num() > 0)
	{
		StoreLastMorphTarget();
	}
	
	UDynamicMesh* NewMorph = NewObject<UDynamicMesh>(this);
	
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
	NewMorph = UGeometryScriptLibrary_MeshDecompositionFunctions::CopyMeshToMesh(NewMorph, FindComponentByClass<UDynamicMeshComponent>()->GetDynamicMesh(), OutMesh);

	MorphTargetMeshMap.Add(MorphTargetMeshName, NewMorph);
	
}

void AMorphTargetCreatorProxyActor::StoreLastMorphTarget()
{
	TArray<FName> MorphTargetMeshNames;
	MorphTargetMeshMap.GetKeys(MorphTargetMeshNames);
	const FName MorphTargetMeshNameKey = MorphTargetMeshNames.Last();

	if (MorphTargetMeshMap.Contains(MorphTargetMeshNameKey))
	{
		UDynamicMesh* OutMesh;
		UGeometryScriptLibrary_MeshDecompositionFunctions::CopyMeshToMesh
		(
			FindComponentByClass<UDynamicMeshComponent>()->GetDynamicMesh(),
			MorphTargetMeshMap[MorphTargetMeshNameKey],
			OutMesh
		);

		//MorphTargetMeshMap[MorphTargetMeshNameKey] = OutMesh;
	}
}

void AMorphTargetCreatorProxyActor::RestoreLastMorphTarget()
{
	TArray<FName> MorphTargetMeshNames;
	MorphTargetMeshMap.GetKeys(MorphTargetMeshNames);
	const FName MorphTargetMeshNameKey = MorphTargetMeshNames.Last();

	if (MorphTargetMeshMap.Contains(MorphTargetMeshNameKey))
	{
		UDynamicMesh* OutMesh;
		UGeometryScriptLibrary_MeshDecompositionFunctions::CopyMeshToMesh
		(
			MorphTargetMeshMap[MorphTargetMeshNameKey],
			FindComponentByClass<UDynamicMeshComponent>()->GetDynamicMesh(),
			OutMesh
		);

		//MorphTargetMeshMap[MorphTargetMeshNameKey] = OutMesh;
	}
}

void AMorphTargetCreatorProxyActor::SaveObject(UDynamicMesh* TargetMesh)
{
	// Duplicate this asset in the same file path its in
	
	FGeometryScriptCopyMorphTargetToAssetOptions CopyToMeshOptions;
	CopyToMeshOptions.bOverwriteExistingTarget = true;
	CopyToMeshOptions.bEmitTransaction = true;
	EGeometryScriptOutcomePins CopyToMeshOutcome;
	

	FGeometryScriptMeshWriteLOD LodSettings;
	LodSettings.LODIndex = 0;

	// Copy the last mesh version into the last morph target entry;
	StoreLastMorphTarget();

	
	for (const auto Morph : MorphTargetMeshMap)
	{
		auto CopiedMesh = UGeometryScriptLibrary_StaticMeshFunctions::CopyMorphTargetToSkeletalMesh(Morph.Value, MorphTargetMesh, Morph.Key, CopyToMeshOptions, LodSettings , CopyToMeshOutcome);
	}

	
	
	bShouldStoreMeshIntoAsset = true;


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

	RerunConstructionScripts();
}

