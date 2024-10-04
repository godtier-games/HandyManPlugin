// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/DataTypes/HandyManDataTypes.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "ScatterGeometryTool.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UScatterGeometryTool : public UHandyManSingleClickTool
{
	GENERATED_BODY()
public:
	UScatterGeometryTool();

	virtual void Setup() override;

	// Click
	//virtual FInputRayHit TestIfHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;
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

	AActor* SpawnNewScatterWorldActor(const AActor* ActorToSpawnOn);
	
	UPROPERTY()
	class UScatterGeometryTool_PropertySet* PropertySet;

	UPROPERTY()
	TMap<AActor*, FObjectSelection> SelectedActors;

	void HighlightActors(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection = false);
	void HighlightSelectedActor(const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection, const FHitResult& HitResult);


	

};


UCLASS()
class HANDYMAN_API UScatterGeometryTool_PropertySet : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	UScatterGeometryTool_PropertySet();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "Change the global mesh data of the entire PCG Graph instance for each tool in the level."))
	class UHM_ScatterToolGeometryData* MeshData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "If it seems your Ivy is not generating it may be due to a small amount of triangles in the mesh. Toggle voxelization to generate more points."))
	bool bVoxelizeMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (EditCondition = "bUseVoxelization", EditConditionHides, ClampMin = "0.1", Tooltip = "The lower this value is the more points will be generated. This will increase cook times and memory usage."))
	float VoxelSize = 20.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "Change the global seed of the entire PCG Graph instance for each tool in the level."))
	int32 RandomSeed = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Tooltip = "This will determine the random scale of the  meshes that are spawned on the vine."))
	FVector2D GeometryScaleRange = FVector2D(1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bUseRandomRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (EditCondition = "bUseRandomRotation"))
	FRotator MinRandomRotation = FRotator();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (EditCondition = "bUseRandomRotation"))
	FRotator MaxRandomRotation = FRotator();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bUseRandomOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (EditCondition = "bUseRandomOffset"))
	FVector MinRandomOffset = FVector();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (EditCondition = "bUseRandomOffset"))
	FVector MaxRandomOffset = FVector();

	

	
};
