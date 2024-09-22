// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryScript/MeshDeformFunctions.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "IslandGeneratorTool.generated.h"

class ARuntimeIslandGenerator;
class UIslandGeneratorPropertySet;
/**
 * 
 */
UCLASS()
class HANDYMAN_API UIslandGeneratorTool : public UHandyManSingleClickTool
{
	GENERATED_BODY()

public:
	UIslandGeneratorTool();

	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	///~ Gizmo Behavior
	virtual void OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform, EScriptableToolGizmoStateChangeType ChangeType) override;
	virtual void OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform) override;

	void SpawnOutputActorInstance(const UIslandGeneratorPropertySet* InSettings, const FTransform& SpawnTransform = FTransform::Identity);


private:

	/** Last selected actor list */
	UPROPERTY()
	ARuntimeIslandGenerator* OutputActor;
	

	UPROPERTY()
	class UIslandGeneratorPropertySet* Settings;
};



UCLASS()
class HANDYMAN_API UIslandGeneratorPropertySet : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Island Generation")
	bool bShouldGenerateOnConstruction = true;

	UPROPERTY(EditAnywhere, Category = "Island Generation", meta=(UIMin = 0, UIMax = 10000))
	int32 RandomSeed = 0;

	UPROPERTY(EditAnywhere, Category = "Island Generation", meta=(UIMin = 3, UIMax = 50, ClampMin = 3, ClampMax = 100))
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

	UPROPERTY(EditAnywhere, Category = "Island Generation")
	bool bUseHeightMap = false;

	UPROPERTY(EditAnywhere, Category = "Island Generation", meta=(EditCondition="bUseHeightMap", EditConditionHides))
	TObjectPtr<UTexture2D> HeightMap;

	UPROPERTY(EditAnywhere, Category = "Island Generation", meta=(EditCondition="bUseHeightMap", EditConditionHides))
	FGeometryScriptDisplaceFromTextureOptions DisplaceOptions;
};


