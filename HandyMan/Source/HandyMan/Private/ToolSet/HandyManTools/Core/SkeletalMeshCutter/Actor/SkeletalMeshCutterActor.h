// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryActors/GeneratedDynamicMeshActor.h"
#include "GeometryScript/GeometryScriptSelectionTypes.h"
#include "ToolSet/HandyManTools/Core/SkeletalMeshCutter/DataTypes/SkeletalMeshCutterTypes.h"
#include "SkeletalMeshCutterActor.generated.h"


UCLASS()
class HANDYMAN_API ASkeletalMeshCutterActor : public AGeneratedDynamicMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASkeletalMeshCutterActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void RebuildGeneratedMesh(UDynamicMesh* TargetMesh) override;

	FVector AddNewCutter(const uint8& Index, const ECutterShapeType& Shape);
	void RemoveCutter(const uint8& Index);

	void RemoveCreatedAsset();

	


#if WITH_EDITOR
	void Initialize(const FSkeletalMeshAssetData& MeshData);
	void SaveObject();
#endif
	
	


protected:



#if WITH_EDITORONLY_DATA

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USkeletalMeshLODSettings> CutMeshSettings;
	
	UPROPERTY()
	TObjectPtr<USkeletalMesh> InputMesh = nullptr;

	UPROPERTY()
	TObjectPtr<USkeletalMesh> SavedSkeletalMesh = nullptr;

	UPROPERTY()
	TObjectPtr<USkeletalMesh> LastSkeletalMesh = nullptr;

	UPROPERTY()
	FSkeletalMeshAssetData CacheMeshData;

	UPROPERTY()
	TMap<uint8, ECutterShapeType> Cutters;

	int32 EditNum = 0;
	
	FGeometryScriptMeshSelection CombinedGeometrySelection;
	bool bShouldStoreMeshIntoAsset = false;
#endif
	
};
