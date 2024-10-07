// Fill out your copyright notice in the Description page of Project Settings.


#include "ExtractMeshProxyActor.h"

#include "EditorAssetLibrary.h"
#include "MeshInspectorTool.h"
#include "GeometryScript/CreateNewAssetUtilityFunctions.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshDecompositionFunctions.h"
#include "GeometryScript/MeshMaterialFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"


// Sets default values
AExtractMeshProxyActor::AExtractMeshProxyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AExtractMeshProxyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AExtractMeshProxyActor::RebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{ 
	if (ExtractedMeshes.Num() == 0) return;

	TargetMesh->Reset();
	ReleaseAllComputeMeshes();
	// Extract each material ID

	bool bShouldOffsetToRight = false;
	// Display the mesh for only mesh lod 0 and take in account the offset set by the user
	for (int i = 0; i < ExtractedMeshes.Num(); ++i)
	{
		const auto& ExtractedMesh = ExtractedMeshes[i];

		if(ExtractedMesh.MeshLods.Num() == 0) continue;

		bShouldOffsetToRight = !bShouldOffsetToRight;
		const float multiplier = bShouldOffsetToRight ? 1.0f : -1.0f;

		UDynamicMesh* Copy = AllocateComputeMesh();
		// Copy mesh into allocated mesh then append that mesh with the offset
		UDynamicMesh* OutMesh;
		UGeometryScriptLibrary_MeshDecompositionFunctions::CopyMeshToMesh(ExtractedMesh.MeshLods[0], Copy, OutMesh);

		UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(TargetMesh, Copy, FTransform(FVector(i * MeshOffset * multiplier, 0, 0)));
		
		if (ExtractedMesh.MaterialID)
		{
			GetDynamicMeshComponent()->SetMaterial(i, ExtractedMesh.MaterialID);
		}
	}

	
	
	Super::RebuildGeneratedMesh(TargetMesh);
}

// Called every frame
void AExtractMeshProxyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AExtractMeshProxyActor::SetMeshOffset(const float NewOffset)
{
	MeshOffset = NewOffset;
	RerunConstructionScripts();
}

void AExtractMeshProxyActor::UpdateExtractedInfo(const TArray<FExtractedMeshInfo>& NewInfo)
{
	ExtractedMeshes.Empty();
	ExtractedMeshes = NewInfo;
	RerunConstructionScripts();
}

void AExtractMeshProxyActor::SaveObjects(const TArray<FExtractedMeshInfo>& Meshes, const FString& FolderName, const FString& MergedAssetName, const bool MergeMeshes, const bool bExtractAsStaticMesh)
{
	// Iterate over the input meshes, create a new asset with the asset name
	if(!InputMesh) return;
	
	const auto AssetData = FAssetData(InputMesh, false);

	TMap<UMaterialInterface*, FMaterialIDRemap> MaterialIDRemap;
	
	if (!MergeMeshes)
	{

		for (int j = 0; j < Meshes.Num(); ++j)
		{
			const auto& Mesh = Meshes[j];
			if(!Mesh.MaterialID) continue;
			if(Mesh.CustomMeshName.IsNone()) continue;

			for (int32 Idx = 0; Idx < InputMesh->GetMaterials().Num(); ++Idx)
			{
				const auto& Material = InputMesh->GetMaterials()[Idx].MaterialInterface;
				if (Mesh.MaterialID != Material)
				{
					continue;
				}
				FMaterialIDRemap Remap;
				Remap.Original = Idx;
				Remap.Remap = 0;
				MaterialIDRemap.Add(Mesh.MaterialID, Remap);
				break;
			}

			if(!MaterialIDRemap.Contains(Mesh.MaterialID)) continue;

			const auto& IdRemap = MaterialIDRemap[Mesh.MaterialID];

			// Create a new asset
			// Duplicate this asset in the same file path its in
		
			const FString AssetSourcePath = AssetData.GetSoftObjectPath().ToString();
			const FString NewSavePath = FString::Printf(TEXT("%s/%s/%s_%s"), *AssetData.PackagePath.ToString(), *FolderName, *AssetData.AssetName.ToString(), *Mesh.CustomMeshName.ToString());

			FGeometryScriptCopyMeshToAssetOptions CopyToMeshOptions;
			CopyToMeshOptions.bEnableRecomputeNormals = true;
			CopyToMeshOptions.bEnableRecomputeTangents = true;
			CopyToMeshOptions.bUseOriginalVertexOrder = true;
			CopyToMeshOptions.bReplaceMaterials = true;
			CopyToMeshOptions.NewMaterials = {Mesh.MaterialID};

			EGeometryScriptOutcomePins Outcome;
			if (!bExtractAsStaticMesh)
			{
				if (USkeletalMesh* DuplicatedMesh = Cast<USkeletalMesh>(UEditorAssetLibrary::DuplicateAsset(AssetSourcePath, NewSavePath)))
				{
	
					UEditorAssetLibrary::SaveAsset(NewSavePath, false);
					
					// Copy this dynamic mesh into this new asset
					for (int i = 0; i < Mesh.MeshLods.Num(); ++i)
					{
				
						auto& LOD = Mesh.MeshLods[i];

						UGeometryScriptLibrary_MeshMaterialFunctions::RemapMaterialIDs(LOD, IdRemap.Original, IdRemap.Remap);
				
						FGeometryScriptMeshWriteLOD LodSettings;
						LodSettings.LODIndex = i;
						UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToSkeletalMesh(LOD, DuplicatedMesh, CopyToMeshOptions, LodSettings, Outcome);
					}
			
				}
			}
			else
			{
				
				FGeometryScriptCreateNewStaticMeshAssetOptions CreateNewAssetOptions;
				CreateNewAssetOptions.bUseOriginalVertexOrder = true;
				CreateNewAssetOptions.bEnableRecomputeNormals = true;
				CreateNewAssetOptions.bEnableRecomputeTangents = true;
				
				auto GeneratedAsset = UGeometryScriptLibrary_CreateNewAssetFunctions::CreateNewStaticMeshAssetFromMesh(Mesh.MeshLods[0], NewSavePath, CreateNewAssetOptions, Outcome );

				UEditorAssetLibrary::SaveAsset(NewSavePath, false);

				if (GeneratedAsset)
				{
					for (int i = 0; i < Mesh.MeshLods.Num(); ++i)
					{
				
						auto& LOD = Mesh.MeshLods[i];

						UGeometryScriptLibrary_MeshMaterialFunctions::RemapMaterialIDs(LOD, IdRemap.Original, IdRemap.Remap);

						FGeometryScriptMeshWriteLOD LodSettings;
						LodSettings.LODIndex = i;

						UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToStaticMesh(LOD, GeneratedAsset, CopyToMeshOptions, LodSettings, Outcome, false);
					
					}
				}
				
				ReleaseAllComputeMeshes();
				
			}
		
		}
	}
	else
	{
		const FString AssetSourcePath = AssetData.GetSoftObjectPath().ToString();
		const FString NewSavePath = FString::Printf(TEXT("%s/%s/%s_%s"), *AssetData.PackagePath.ToString(), *FolderName, *AssetData.AssetName.ToString(), *MergedAssetName);
		
		TArray<UDynamicMesh*> MeshLODs;

	

		for (int i = 0; i < Meshes.Num(); ++i)
		{
			const auto& Mesh = Meshes[i];
			// get the original mesh ID and map it to the index of the query
			for (int32 Idx = 0; Idx < InputMesh->GetMaterials().Num(); ++Idx)
			{
				const auto& Material = InputMesh->GetMaterials()[Idx].MaterialInterface;
				if (Mesh.MaterialID != Material)
				{
					continue;
				}
				FMaterialIDRemap Remap;
				Remap.Original = Idx;
				Remap.Remap = i;
				MaterialIDRemap.Add(Mesh.MaterialID, Remap);
				break;
			}
		}


		for (int i = 0; i < InputMesh->GetLODNum(); ++i)
		{
			
			UDynamicMesh* CombinedLOD = NewObject<UDynamicMesh>();
			for (const auto& Mesh : Meshes)
			{
				UDynamicMesh* MeshLod = Mesh.MeshLods[i];
				if(!MeshLod) continue;
				if(!MaterialIDRemap.Contains(Mesh.MaterialID)) continue;

				const auto& IdRemap = MaterialIDRemap[Mesh.MaterialID];
				
				UGeometryScriptLibrary_MeshMaterialFunctions::RemapMaterialIDs(MeshLod, IdRemap.Original, IdRemap.Remap);
					
				UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(CombinedLOD, MeshLod, FTransform::Identity);
			}

			MeshLODs.Add(CombinedLOD);
		}

		
		/*FGeometryScriptCreateNewSkeletalMeshAssetOptions CreateNewOptions;
		CreateNewOptions.bEnableRecomputeNormals = true;
		CreateNewOptions.bEnableRecomputeTangents = true;
		CreateNewOptions.bUseOriginalVertexOrder = true;
		EGeometryScriptOutcomePins CreateNewMeshOutcome;

		for (const auto& Material : DynamicMeshComponent->GetMaterialSlotNames())
		{
			CreateNewOptions.Materials.Add(Material, DynamicMeshComponent->GetMaterialByName(Material));
		}
			
		UGeometryScriptLibrary_CreateNewAssetFunctions::CreateNewSkeletalMeshAssetFromMeshLODs(MeshLODs, InputMesh->GetSkeleton(), NewSavePath, CreateNewOptions, CreateNewMeshOutcome);*/

		EGeometryScriptOutcomePins Outcome;
		
		if (!bExtractAsStaticMesh)
		{
			if (USkeletalMesh* DuplicatedMesh = Cast<USkeletalMesh>(UEditorAssetLibrary::DuplicateAsset(AssetSourcePath, NewSavePath)))
			{
	
				UEditorAssetLibrary::SaveAsset(NewSavePath, false);

				TArray<UMaterialInterface*> Materials;
				MaterialIDRemap.GetKeys(Materials);
				FGeometryScriptCopyMeshToAssetOptions CopyToMeshOptions;
				CopyToMeshOptions.bEnableRecomputeNormals = true;
				CopyToMeshOptions.bEnableRecomputeTangents = true;
				CopyToMeshOptions.bUseOriginalVertexOrder = true;
				CopyToMeshOptions.bReplaceMaterials = true;
				CopyToMeshOptions.NewMaterials = Materials;
				

			
			
				// Copy this dynamic mesh into this new asset
				for (int i = 0; i < MeshLODs.Num(); ++i)
				{
				
					auto& LOD = MeshLODs[i];
				
				
					FGeometryScriptMeshWriteLOD LodSettings;
					LodSettings.LODIndex = i;
					UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToSkeletalMesh(LOD, DuplicatedMesh, CopyToMeshOptions, LodSettings, Outcome);
				}
			
			}
		}
		else
		{
			FGeometryScriptCreateNewStaticMeshAssetOptions CreateNewAssetOptions;
			CreateNewAssetOptions.bUseOriginalVertexOrder = true;
			CreateNewAssetOptions.bEnableRecomputeNormals = true;
			CreateNewAssetOptions.bEnableRecomputeTangents = true;

			auto GeneratedAsset = UGeometryScriptLibrary_CreateNewAssetFunctions::CreateNewStaticMeshAssetFromMeshLODs(MeshLODs, NewSavePath, CreateNewAssetOptions, Outcome );
			ReleaseAllComputeMeshes();
				
		}
	}

	ReleaseAllComputeMeshes();
	SetIsTemporarilyHiddenInEditor(true);
	SetLifeSpan(1.0f);
}

void AExtractMeshProxyActor::ExtractMeshInfo(USkeletalMesh* Mesh, TArray<FExtractedMeshInfo>& OutExtractedMeshes)
{
	if(!Mesh) return;

	InputMesh = Mesh;

	for (int j = 0; j < InputMesh->GetNumMaterials(); ++j)
	{
		const auto Material = InputMesh->GetMaterials()[j];
		FExtractedMeshInfo MeshInfo;
		MeshInfo.MaterialID = Material.MaterialInterface;
		MeshInfo.CustomMeshName = Material.MaterialSlotName;

		for (int i = 0; i < InputMesh->GetLODNum(); ++i)
		{
			// Extract the mesh for this material ID and store it in a dynamic mesh
			EGeometryScriptOutcomePins Outcome;
			UDynamicMesh* LodMesh = NewObject<UDynamicMesh>(this);
			FGeometryScriptCopyMeshFromAssetOptions CopyFromAssetOptions;
			CopyFromAssetOptions.bApplyBuildSettings = false;
			FGeometryScriptMeshReadLOD ReadLOD;
			ReadLOD.LODIndex = i;
			UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromSkeletalMesh(InputMesh, LodMesh, CopyFromAssetOptions, ReadLOD, Outcome);


			FGeometryScriptMeshSelection MeshSelectionToInvert;
			UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsByMaterialID(LodMesh, j, MeshSelectionToInvert);

			FGeometryScriptMeshSelection MeshSelectionToDestroy;
			UGeometryScriptLibrary_MeshSelectionFunctions::InvertMeshSelection(LodMesh, MeshSelectionToInvert, MeshSelectionToDestroy);

			int TrianglesDestroyed;
			UGeometryScriptLibrary_MeshBasicEditFunctions::DeleteSelectedTrianglesFromMesh(LodMesh, MeshSelectionToDestroy, TrianglesDestroyed);

			MeshInfo.MeshLods.Add(LodMesh);
		}

		OutExtractedMeshes.Add(MeshInfo); // Store array in the tool this will be used for editing the names of the mesh
		ExtractedMeshes.Add(MeshInfo); // Store array locally in the actor this will be used to display the mesh.
	}

	RerunConstructionScripts();
}

UStaticMesh* AExtractMeshProxyActor::MakeStaticMesh(USkeletalMesh* SkeletalMesh, const FString& PackageName)
{
	UDynamicMesh* Copy = AllocateComputeMesh();

	FGeometryScriptCopyMeshFromAssetOptions CopyOptions;
	CopyOptions.bApplyBuildSettings = false;

	FGeometryScriptMeshReadLOD CopyLod;
	EGeometryScriptOutcomePins Outcome;
	
	UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromSkeletalMesh(SkeletalMesh, Copy, CopyOptions, CopyLod, Outcome);

	FGeometryScriptCreateNewStaticMeshAssetOptions CreateNewAssetOptions;
	CreateNewAssetOptions.bUseOriginalVertexOrder = true;

	auto GeneratedAsset = UGeometryScriptLibrary_CreateNewAssetFunctions::CreateNewStaticMeshAssetFromMesh(Copy, PackageName, CreateNewAssetOptions, Outcome);;
	ReleaseAllComputeMeshes();

	for (int i = 0; i < SkeletalMesh->GetMaterials().Num(); ++i)
	{
		GeneratedAsset->SetMaterial(i, SkeletalMesh->GetMaterials()[i].MaterialInterface);
	}
	return GeneratedAsset;
}

