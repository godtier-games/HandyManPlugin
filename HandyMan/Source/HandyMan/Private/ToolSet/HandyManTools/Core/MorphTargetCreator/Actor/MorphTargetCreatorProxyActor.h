// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MorphTargetCreatorProxyActor.generated.h"

class UDynamicMesh;

UCLASS()
class HANDYMAN_API AMorphTargetCreatorProxyActor : public AActor
{
	GENERATED_BODY()

public:
	
/*#if WITH_EDITOR
	virtual bool IsSelectable() const override
	{
		return false;
	}
#endif*/
	
	// Sets default values for this actor's properties
	AMorphTargetCreatorProxyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;


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
	void RemoveMorphTargetMesh(const FName& MorphTargetMeshName, const bool bShouldRestoreMesh = false);
	void RemoveAllMorphTargetMeshes();
	void CreateMorphTargetMesh(const FName& MorphTargetMeshName);
	void StoreLastMorphTarget();
	void RestoreLastMorphTarget();
	void SaveObject(UDynamicMesh* TargetMesh);




private:
	UPROPERTY()
	UDynamicMesh* BaseMesh = nullptr;

	bool bShouldStoreMeshIntoAsset = false;

};
