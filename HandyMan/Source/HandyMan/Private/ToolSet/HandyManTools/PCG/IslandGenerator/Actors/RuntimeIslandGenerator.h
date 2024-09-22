// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_DynamicMeshActor_Runtime.h"
#include "RuntimeIslandGenerator.generated.h"

UCLASS()
class HANDYMAN_API ARuntimeIslandGenerator : public APCG_DynamicMeshActor_Runtime
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARuntimeIslandGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY(EditAnywhere, Category = "Island Generation")
	bool bShouldGenerateOnConstruction = true;

	UPROPERTY(EditAnywhere, Category = "Island Generation")
	int32 RandomSeed = 0;

	UPROPERTY(EditAnywhere, Category = "Island Generation")
	int32 IslandChunks = 17;

	UPROPERTY(EditAnywhere, Category = "Island Generation")
	FVector2D IslandBounds = FVector2D(800.0f, 5000.0f);
	
	UPROPERTY(EditAnywhere, Category = "Island Generation")
	float MaxSpawnArea = 9000.0f;

	UPROPERTY(EditAnywhere, Category = "Island Generation")
	float IslandDepth = -800.0f;

	UPROPERTY(EditAnywhere, Category = "Island Generation")
	bool bShouldFlattenIsland = true;

	UPROPERTY(EditAnywhere, Category = "Island Generation")
	TObjectPtr<UMaterialParameterCollection> MaterialParameterCollection;

	UPROPERTY(EditAnywhere, Category = "Island Generation")
	FLinearColor GrassColor = FLinearColor::Green;

public:
	void SetShouldGenerateOnConstruction(const bool ShouldGenerate);
	void SetRandomSeed(const int32 InRandomSeed);
	void SetIslandChunkCount(const int32 InCount);
	void SetIslandBounds(const FVector2D& InBounds);
	void SetMaxSpawnArea(const float SpawnArea);
	void SetIslandDepth(const float Depth);
	void SetShouldFlattenIsland(const bool ShouldFlatten);
	void SetMaterialParameterCollection(const TObjectPtr<UMaterialParameterCollection>& NewCollection);
	void SetGrassColor(const FLinearColor& Color);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void GenerateIsland();

	UFUNCTION(BlueprintPure)
	int32 PlatformSwitch(const int32 LowEnd, const int32 HighEnd) const;
	
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor)
	void RegenerateIsland();

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
