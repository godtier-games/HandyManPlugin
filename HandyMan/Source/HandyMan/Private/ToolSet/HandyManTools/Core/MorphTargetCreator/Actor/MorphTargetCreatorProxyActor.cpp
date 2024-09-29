// Fill out your copyright notice in the Description page of Project Settings.


#include "MorphTargetCreatorProxyActor.h"

#include "EditorAssetLibrary.h"
#include "SkeletalMeshAttributes.h"
#include "UDynamicMesh.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/DynamicMeshComponent.h"
#include "DynamicMesh/NonManifoldMappingSupport.h"
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

void AMorphTargetCreatorProxyActor::RemoveMorphTargetMesh(const FName& MorphTargetMeshName)
{
	if(MorphTargetMeshName.IsEqual(NAME_None)) return;

	if(!MorphTargetMeshMap.Contains(MorphTargetMeshName)) return;

	MorphTargetMeshMap.Remove(MorphTargetMeshName);

	
}

void AMorphTargetCreatorProxyActor::RemoveAllMorphTargetMeshes(const bool bShouldRestoreMesh)
{
	for (const auto MeshMap : MorphTargetMeshMap)
	{
		RemoveMorphTargetMesh(MeshMap.Key);
	}

	if (bShouldRestoreMesh)
	{
		RestoreLastMorphTarget();
	}
	else
	{
		UDynamicMesh* OutMesh;
		UGeometryScriptLibrary_MeshDecompositionFunctions::CopyMeshToMesh(BaseMesh, FindComponentByClass<UDynamicMeshComponent>()->GetDynamicMesh(), OutMesh);
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

void AMorphTargetCreatorProxyActor::CloneMorphTarget(const FName& MorphTargetName, const FName& NewMorphTargetName)
{

	if(!MorphTargetMesh) return;

	// Copy the last morph mesh into the object we previously created
	if (MorphTargetMeshMap.Num() > 0)
	{
		StoreLastMorphTarget();
	}
	
	UDynamicMesh* Mesh = FindComponentByClass<UDynamicMeshComponent>()->GetDynamicMesh();
	FMeshDescription* MeshDescription = MorphTargetMesh->GetMeshDescription(0);
	
	if(!Mesh || MorphTargetName.IsEqual(NAME_None) || MeshDescription == nullptr) return;

	FSkeletalMeshAttributes MeshAttributes(*MeshDescription);
	MeshAttributes.Register();
	
	if (!MeshAttributes.GetMorphTargetNames().Contains(MorphTargetName))
	{
		return;
	}
	
	FGeometryScriptCopyMeshFromAssetOptions CopyFromAssetOptions;
	CopyFromAssetOptions.bRequestTangents = true;
	CopyFromAssetOptions.bApplyBuildSettings = false;

	FGeometryScriptMeshReadLOD MeshRead;
	MeshRead.LODType = EGeometryScriptLODType::HiResSourceModel;

	EGeometryScriptOutcomePins CopyFromAssetOutcome;
	UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromSkeletalMesh(MorphTargetMesh, Mesh, CopyFromAssetOptions, MeshRead, CopyFromAssetOutcome);
	
	const FSkeletalMeshLODInfo* LODInfo = MorphTargetMesh->GetLODInfo(0);
	const float MorphThresholdSquared = LODInfo->BuildSettings.MorphThresholdPosition * LODInfo->BuildSettings.MorphThresholdPosition;
	
	/*TVertexAttributesRef<FVector3f> PositionDelta = MeshAttributes.GetVertexMorphPositionDelta(MorphTargetName);
	TVertexAttributesRef<FVector3f> VertexPositions = MeshAttributes.GetVertexPositions();*/

	
	TVertexAttributesRef<FVector3f> MorphTargetPositions = MeshAttributes.GetVertexMorphPositionDelta(MorphTargetName);


	const FDynamicMesh3& SourceMesh = Mesh->GetMeshRef();
	const UE::Geometry::FNonManifoldMappingSupport NonManifoldMappingSupport(SourceMesh);
	int32 SourceVertexCount;

	if (NonManifoldMappingSupport.IsNonManifoldVertexInSource())
	{
		TSet<int32> UniqueVertices;
		for (int32 SourceVID = 0; SourceVID < SourceMesh.VertexCount(); ++SourceVID)
		{
			UniqueVertices.Add(NonManifoldMappingSupport.GetOriginalNonManifoldVertexID(SourceVID));
		}

		SourceVertexCount = UniqueVertices.Num();
	}
	else
	{
		SourceVertexCount = SourceMesh.VertexCount();
	}
	if (MeshDescription->Vertices().Num() != SourceVertexCount)
	{
		return;
	}
	
	for (int32 SourceVID = 0; SourceVID < SourceMesh.VertexCount(); ++SourceVID)
	{
		const int32 TargetVID = NonManifoldMappingSupport.GetOriginalNonManifoldVertexID(SourceVID);
		
		const FVector3f V0 = MorphTargetPositions[TargetVID];
		const FVector3d V1 = Mesh->GetMeshRef().GetVertex(SourceVID);

		const FVector3f Delta = V0 + FVector3f{V1};
		if (Delta.SquaredLength() > MorphThresholdSquared)
		{
			Mesh->GetMeshRef().SetVertex(TargetVID, (FVector3d)Delta);
		}
	}

	UDynamicMesh* NewMorph = NewObject<UDynamicMesh>(this);
	UDynamicMesh* OutMesh;
	UGeometryScriptLibrary_MeshDecompositionFunctions::CopyMeshToMesh
	(
		FindComponentByClass<UDynamicMeshComponent>()->GetDynamicMesh(),
		NewMorph,
		OutMesh
	);

	if (NewMorphTargetName.IsEqual(NAME_None))
	{
		MorphTargetMeshMap.Add(MorphTargetName, NewMorph);
	}
	else
	{
		MorphTargetMeshMap.Add(NewMorphTargetName, NewMorph);
	}

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
	MorphTargetMesh = nullptr;
	SetIsTemporarilyHiddenInEditor(true);
	SetLifeSpan(2.f);

	RerunConstructionScripts();
}

