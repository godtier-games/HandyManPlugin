// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryScript/GeometryScriptTypes.h"
#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_DynamicMeshActor_Editor.h"
#include "ToolSet/HandyManTools/PCG/SplineTool/DynamicCollider/Actor/PCG_DynamicSplineActor.h"
#include "PCG_BuildingGenerator.generated.h"


/**
 *  This actor generates a building based on a block out mesh. The mesh generated is completely procedural and can be modified by changing the parameters in the PCG component.
 *  There is also the option to bake this mesh to a static mesh asset. Which should be done to reduce the overhead of the procedural generation.
 */
UCLASS(PrioritizeCategories="Parameters")
class HANDYMAN_API APCG_BuildingGenerator : public APCG_DynamicMeshActor_Editor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APCG_BuildingGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	virtual void RebuildGeneratedMesh(UDynamicMesh* TargetMesh) override;

	virtual UPCGComponent* GetPCGComponent() const override { return PCG; }

	// Sets default values for this actor's properties

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetBuildingMaterial(const TSoftObjectPtr<UMaterialInterface> Material) {BuildingMaterial = Material;}

	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void CacheInputActor(AActor* InputActor); 

	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetFloorMaterial(const int32 Floor, const TSoftObjectPtr<UMaterialInterface> Material);

	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetNumberOfFloors(const int32 NewFloorCount);
	
	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetHasOpenRoof(const bool HasOpenRoof);

	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetUseConsistentFloorMaterials(const bool UseConsistentFloorMaterials);

	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetUseConsistentFloorHeight(const bool UseConsistentFloorHeight);

	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetDesiredFloorHeight(const float NewFloorHeight);

	UFUNCTION(BlueprintCallable, Category ="Handy Man")
	void SetDesiredBuildingHeight(const float NewBuildingHeight);
	
	
	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetWallThickness(const float Thickness) {WallThickness = Thickness;};

	/*UFUNCTION(BlueprintCallable, Category="Handy Man")
	void TransferMeshMaterials(const TArray<UMaterialInterface*> Materials);*/


	
	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void CreateSplinesFromPolyPaths(const TArray<FGeometryScriptPolyPath> Paths);
	
	/*UFUNCTION(meta=(CallInEditor="true"))
	void BakeToStaticMesh();*/
	


protected:

	void ForceCookPCG();

	FDelegateHandle DelegateHandle;

	UPROPERTY(BlueprintReadWrite, Category="Inputs")
	bool bRefreshDelegates = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPCGComponent> PCG;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters")
	bool bHasOpenRoof = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters")
	TSoftObjectPtr<UMaterialInterface> BuildingMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters")
	bool bUseConsistentFloorMaterial = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters", meta=(EditCondition="bUseConsistentFloorMaterial"))
	TSoftObjectPtr<UMaterialInterface> FloorMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters", meta=(EditCondition="!bUseConsistentFloorMaterial"))
	TMap<uint8, TSoftObjectPtr<UMaterialInterface>> FloorMaterialMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters")
	float WallThickness = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters")
	bool bUseConsistentFloorHeight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters", meta=(EditCondition="bUseConsistentFloorHeight"))
	float DesiredFloorHeight = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters", meta=(EditCondition="!bUseConsistentFloorHeight"))
	TMap<uint8, float> DesiredFloorHeightMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters")
	int32 NumberOfFloors = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters")
	float BuildingHeight = 500.f;

private:

	UPROPERTY()
	TMap<uint8, TObjectPtr<USplineComponent>> FloorSplines;
	
	UPROPERTY()
	TArray<FGeometryScriptPolyPath> FloorPolyPaths;

	UPROPERTY()
	UDynamicMesh* OriginalMesh = nullptr;

	UPROPERTY()
	AActor* OriginalActor = nullptr;


	void GenerateRoofMesh(UDynamicMesh* TargetMesh);
	void GenerateFloorMeshes(UDynamicMesh* TargetMesh);
	void GenerateExteriorWalls(UDynamicMesh* TargetMesh);

	
};
