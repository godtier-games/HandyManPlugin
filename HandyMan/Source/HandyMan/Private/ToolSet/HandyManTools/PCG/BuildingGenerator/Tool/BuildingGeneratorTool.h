// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Behaviors/ScriptableToolBehaviorDelegates.h"
#include "ToolSet/HandyManBaseClasses/HandyManClickDragTool.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "BuildingGeneratorTool.generated.h"

class IPCGToolInterface;
class APCG_BuildingGenerator;
class UBuildingGeneratorPropertySet;


/**
 * 
 */
UCLASS()
class HANDYMAN_API UBuildingGeneratorTool : public UHandyManClickDragTool
{
	GENERATED_BODY()

public:

	UBuildingGeneratorTool();

	void CreateBrush();

	void SpawnOutputActorInstance(const UBuildingGeneratorPropertySet* InSettings, const FTransform& SpawnTransform);
	
	virtual void Setup() override;

	// IHoverBehaviorTarget API
	virtual bool OnUpdateHover(const FInputDeviceRay& DevicePos) override;

	virtual void OnClickDrag(const FInputDeviceRay& DragPos) override;

	bool bCanSpawn = false;

	FVector LatestPosition = FVector::Zero();

	UPROPERTY()
	AActor* TargetActor;

	UPROPERTY()
	APCG_BuildingGenerator* OutputActor;

	virtual void OnDragBegin_Implementation(FInputDeviceRay StartPosition, const FScriptableToolModifierStates& Modifiers) override;

	virtual void OnDragEnd_Implementation(FInputDeviceRay EndPosition, const FScriptableToolModifierStates& Modifiers) override;

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
	
	/** Brush actor color */
	FLinearColor BrushColor = FLinearColor::Blue;

	UPROPERTY()
	UStaticMesh* PaintingMesh = nullptr;

	bool bIsPainting = false;
	bool bIsEditing = false;
	bool bIsDestroying = false;
	bool bIsMeshAlignedToSurface = true;
	

	TWeakInterfacePtr<IPCGToolInterface> TargetPCGInterface;

	
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

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters | Building")
	

	
};

