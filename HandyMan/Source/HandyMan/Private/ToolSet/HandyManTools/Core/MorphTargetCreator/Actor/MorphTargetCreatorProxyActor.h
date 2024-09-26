// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_DynamicMeshActor_Editor.h"
#include "MorphTargetCreatorProxyActor.generated.h"

UCLASS()
class HANDYMAN_API AMorphTargetCreatorProxyActor : public APCG_DynamicMeshActor_Editor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMorphTargetCreatorProxyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void RebuildGeneratedMesh(UDynamicMesh* TargetMesh) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	


private:

	UPROPERTY()
	TMap<FName, UDynamicMesh*> MorphTargetMeshMap;

	UPROPERTY()
	USkeletalMesh* MorphTargetMesh;


public:

	void CacheBaseMesh(USkeletalMesh* InputMesh);
	TMap<FName, UDynamicMesh*> GetMorphTargetMeshMap() { return MorphTargetMeshMap; }
	void RemoveMorphTargetMesh(FName MorphTargetMeshName);
	void RemoveAllMorphTargetMeshes();
	void CreateMorphTargetMesh(FName MorphTargetMeshName);
	void SaveObject();




private:
	UPROPERTY()
	UDynamicMesh* BaseMesh = nullptr;

	bool bShouldStoreMeshIntoAsset = false;

};
