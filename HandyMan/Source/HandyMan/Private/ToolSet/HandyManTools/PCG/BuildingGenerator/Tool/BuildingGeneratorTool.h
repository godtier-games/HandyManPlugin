// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Behaviors/ScriptableToolBehaviorDelegates.h"
#include "ToolSet/HandyManBaseClasses/HandyManClickDragTool.h"
#include "ToolSet/HandyManBaseClasses/HandyManModularTool.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "ToolSet/HandyManTools/PCG/BuildingGenerator/DataTypes/BuildingGeneratorTypes.h"
#include "BuildingGeneratorTool.generated.h"

class IPCGToolInterface;
class APCG_BuildingGenerator;
class UBuildingGeneratorPropertySet;

/**
 * 
 */
UCLASS()
class HANDYMAN_API UBuildingGeneratorTool : public UHandyManModularTool
{
	GENERATED_BODY()

public:

	UBuildingGeneratorTool();

	void CreateBrush();

	void SpawnOutputActorInstance(const UBuildingGeneratorPropertySet* InSettings, const FTransform& SpawnTransform);
	
	virtual void Setup() override;


	///~ Hover Behavior
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta=(DisplayName="Can Hover"))
	FInputRayHit TestCanHoverFunc(const FInputDeviceRay& PressPos, const FScriptableToolModifierStates& Modifiers);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Begin Hover"))
	void OnBeginHover(const FInputDeviceRay& DevicePos, const FScriptableToolModifierStates& Modifiers);
	
	///~ IHoverBehaviorTarget API
	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Update Hover"))
	bool OnUpdateHover(const FInputDeviceRay& DevicePos, const FScriptableToolModifierStates& Modifiers);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="On End Hover"))
	void OnEndHover();

	
	///~ Click Drag Behavior
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta=(DisplayName="Can Mouse Drag"))
	FInputRayHit CanClickDrag(const FInputDeviceRay& PressPos, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button);
	
	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Update Mouse Drag"))
	void OnClickDrag(const FInputDeviceRay& DragPos, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button = EScriptableToolMouseButton::LeftButton);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Start Mouse Drag"))
	void OnDragBegin(const FInputDeviceRay& StartPosition, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="On End Mouse Drag"))
	void OnDragEnd(const FInputDeviceRay& EndPosition, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button);


	///~ Single Click Behavior
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta=(DisplayName="Can Click"))
	FInputRayHit CanClickFunc(const FInputDeviceRay& PressPos, const EScriptableToolMouseButton& Button);

	
	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Hit By Click"))
	void OnHitByClickFunc(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& MouseButton);

	UFUNCTION(BlueprintCallable)
	bool MouseBehaviorModiferCheckFunc(const FInputDeviceState& InputDeviceState);


	///~ Mouse Wheel Behavior
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta=(DisplayName="Can Use Mouse Wheel"))
	FInputRayHit CanUseMouseWheel(const FInputDeviceRay& PressPos);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Mouse Wheel Up"))
	void OnMouseWheelUp(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Mouse Wheel Down"))
	void OnMouseWheelDown(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers);


	///~ Gizmo Behavior
	virtual void OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform, EScriptableToolGizmoStateChangeType ChangeType) override;
	virtual void OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform) override;


	
	
	bool bCanSpawn = false;

	FVector LatestPosition = FVector::Zero();

	UPROPERTY()
	AActor* TargetActor;

	UPROPERTY()
	APCG_BuildingGenerator* OutputActor;

	FInputRayHit LastHit;



	virtual void OnTick(float DeltaTime) override;
	
	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	virtual void UpdateAcceptWarnings(EAcceptWarning Warning) override;
	virtual bool CanAccept() const override;
	virtual bool HasCancel() const override {return true;}

	virtual bool Trace(FHitResult& OutHit, const FInputDeviceRay& DevicePos) override;



	
	
	void HandleAccept();
	void HandleCancel();

#pragma region DELEGATES
	
	/** On Level actors added event */
	void OnLevelActorsAdded(AActor* InActor);
	
	/** On level actors deleted event */
	void OnLevelActorsDeleted(AActor* InActor);

	/** On Pre Begin Pie event */
	void OnPreBeginPie(bool InStarted);
	
#pragma endregion

private:

	int32 LastOpeningIndex = 0;

	/** Last place actor world position */
	FVector LastSpawnedPosition;
	
	/** Last selected actor list */
	UPROPERTY()
	AActor* LastSelectedActor;
	
	/** Last placed acotr list */
	UPROPERTY()
	TArray<AActor*> LastSpawnedActors;

	UPROPERTY()
	class UBuildingGeneratorPropertySet* Settings;

	/** Brush actor's material */
	UPROPERTY()
	class UMaterialInstanceDynamic* BrushMI = nullptr;
	
	/** Brush actor */
	UPROPERTY()
	TObjectPtr<class UStaticMeshComponent> Brush = nullptr;

	/** Cursor move world direction for brush actor */
	FVector BrushDirection;
	
	/** Brush actor position */
	FVector BrushPosition;
	
	/** Brush actor rotation */
	FVector BrushNormal;
	
	/** Cursor Last world position */
	FVector BrushLastPosition;

	/** Brush actor rotation */
	FVector BrushScale = FVector(1.f, 1.f, 1.f);
	
	/** Brush actor color */
	FLinearColor BrushColor = FLinearColor::Blue;

	UPROPERTY()
	UStaticMesh* PaintingMesh = nullptr;

	bool bIsPainting = false;
	bool bIsEditing = false;
	bool bIsDestroying = false;
	bool bIsMeshAlignedToSurface = true;
	

	TWeakInterfacePtr<IPCGToolInterface> TargetPCGInterface;

	UPROPERTY()
	TArray<FGeneratedOpening> CachedOpenings;

	
};


UCLASS()
class HANDYMAN_API UBuildingGeneratorPropertySet : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Building")
	TSoftObjectPtr<UMaterialInterface> BuildingMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Building")
	bool bUseConsistentFloorMaterial = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Building", meta=(EditCondition="bUseConsistentFloorMaterial"))
	TSoftObjectPtr<UMaterialInterface> FloorMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Building", meta=(EditCondition="!bUseConsistentFloorMaterial"))
	TMap<uint8, TSoftObjectPtr<UMaterialInterface>> FloorMaterialMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Building")
	float WallThickness = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Building")
	bool bUseConsistentFloorHeight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Building", meta=(EditCondition="bUseConsistentFloorHeight"))
	float DesiredFloorHeight = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Building", meta=(EditCondition="!bUseConsistentFloorHeight"))
	TMap<uint8, float> DesiredFloorHeightMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Building")
	int32 DesiredNumberOfFloors = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Building")
	float DesiredBuildingHeight = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Building")
	bool bHasOpenRoof = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Openings")
	TArray<FDynamicOpening> Openings;
};

