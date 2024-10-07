// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManTools/Core/ExtractMesh/Data/ExtractMeshDataTypes.h"
#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_DynamicMeshActor_Editor.h"
#include "ExtractMeshProxyActor.generated.h"

UCLASS()
class HANDYMAN_API AExtractMeshProxyActor : public APCG_DynamicMeshActor_Editor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AExtractMeshProxyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void RebuildGeneratedMesh(UDynamicMesh* TargetMesh) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetMeshOffset(const float NewOffset);

	TArray<FExtractedMeshInfo> GetExtractedMeshInfo() const {return ExtractedMeshes;};

	void UpdateExtractedInfo(const TArray<FExtractedMeshInfo>& NewInfo);

	void SaveObjects(const TArray<FExtractedMeshInfo>& Meshes, const FString& FolderName, const FString& MergedAssetName = "MERGED", const bool MergeMeshes = false);


	void ExtractMeshInfo(USkeletalMesh* Mesh, TArray<FExtractedMeshInfo>& OutExtractedMeshes);

	
protected:
	UPROPERTY(EditAnywhere, Category="Parameters")
	TArray<FExtractedMeshInfo> ExtractedMeshes;
	
	UPROPERTY(EditAnywhere, Category="Parameters")
	float MeshOffset = 65.0f;


private:

	UPROPERTY()
	TObjectPtr<USkeletalMesh> InputMesh;

	struct FMaterialIDRemap
	{
		int32 Original = 0;
		int32 Remap = 0;
	};

};
