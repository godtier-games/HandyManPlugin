// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/DataTypes/HandyManDataTypes.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "IvyCreator_PCG.generated.h"

class APCG_IvyActor;
/**
 * 
 */
UCLASS()
class HANDYMAN_API UIvyCreator_PCG : public UHandyManSingleClickTool
{
	GENERATED_BODY()
public:
	UIvyCreator_PCG();

	virtual void Setup() override;

	
	virtual void OnHitByClick_Implementation(FInputDeviceRay ClickPos,
	                                                 const FScriptableToolModifierStates& Modifiers) override;

	// Hover API
	virtual void OnHoverBegin_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers) override;
	virtual FInputRayHit OnHoverHitTest_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers) override;
	virtual bool OnHoverUpdate_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers) override;
	virtual void OnHoverEnd_Implementation(const FScriptableToolModifierStates& Modifiers) override;
	virtual void OnTick(float DeltaTime) override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	virtual void UpdateAcceptWarnings(EAcceptWarning Warning) override;
	virtual bool CanAccept() const override;
	virtual bool HasCancel() const override;

	void HandleAccept();
	void HandleCancel();

private:

	AActor* SpawnNewIvyWorldActor(const AActor* ActorToSpawnOn);
	
	UPROPERTY()
	TObjectPtr<class UIvyCreator_PCG_PropertySet> PropertySet;

	UPROPERTY()
	TMap<TObjectPtr<AActor>, FObjectSelection> SelectedActors;

	void HighlightActors(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection = false);
	void HighlightSelectedActor(const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection, const FHitResult& HitResult);


	

};


UCLASS()
class HANDYMAN_API UIvyCreator_PCG_PropertySet : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	UIvyCreator_PCG_PropertySet();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "Change the global mesh data of the entire PCG Graph instance for each tool in the level."))
	TObjectPtr<class UHM_IvyToolMeshData> MeshData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "Show a visual representation of the points being generated on the mesh"))
	bool bDebugMeshPoints = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "If it seems your Ivy is not generating it may be due to a small amount of triangles in the mesh. Toggle voxelization to generate more points."))
	bool bVoxelizeMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (EditCondition = "bUseVoxelization", EditConditionHides, ClampMin = "0.1", Tooltip = "The lower this value is the more points will be generated. This will increase cook times and memory usage."))
	float VoxelSize = 20.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "Change the global seed of the entire PCG Graph instance for each tool in the level."))
	int32 RandomSeed = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "The maximum amount of splines that will be generated by the system."))
	int32 SplineCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "This will determine how far up the meshes the vertical axis will the starting points spawn.", ClampMin = "0.1", ClampMax = "1.0"))
	float StartingPointsHeightRatio = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "Change the global seed of the entire PCG Graph instance for each tool in the level.", ClampMin = "5", ClampMax = "100"))
	int32 PathComplexity = 25;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "This will determine how far up the meshes the vertical axis will the starting points spawn.", UIMin = "0.1", UIMax = "3.0"))
	float VineThickness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "This will determine how far up the meshes the vertical axis will the starting points spawn."))
	float MeshOffsetDistance = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "This will determine the distance between each leaf that is spawned on the vine"))
	float LeafSpawnSpacing = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "This will determine the random scale of the leaf meshes that are spawned on the vine."))
	FVector2D LeafScaleRange = FVector2D(1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bUseRandomLeafRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (EditCondition = "bUseRandomLeafRotation"))
	FRotator MinRandomRotation = FRotator();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (EditCondition = "bUseRandomLeafRotation"))
	FRotator MaxRandomRotation = FRotator();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bUseRandomLeafOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (EditCondition = "bUseRandomLeafOffset"))
	FVector MinRandomOffset = FVector();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (EditCondition = "bUseRandomLeafOffset"))
	FVector MaxRandomOffset = FVector();
};
