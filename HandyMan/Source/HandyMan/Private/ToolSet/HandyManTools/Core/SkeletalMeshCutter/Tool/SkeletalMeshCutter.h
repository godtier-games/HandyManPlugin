// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "ToolSet/HandyManTools/Core/SkeletalMeshCutter/DataTypes/SkeletalMeshCutterTypes.h"
#include "SkeletalMeshCutter.generated.h"

class ASkeletalMeshCutterActor;
class USkeletalMeshCutterPropertySet;
/**
 * 
 */
UCLASS()
class HANDYMAN_API USkeletalMeshCutter : public UHandyManSingleClickTool
{
	GENERATED_BODY()

public:
	USkeletalMeshCutter();

	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual bool CanAccept() const override;

	void SpawnOutputActorInstance(const USkeletalMeshCutterPropertySet* InSettings, const FTransform& SpawnTransform);

	virtual FInputRayHit TestIfHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;
	virtual void OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;
	
	virtual void OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform) override;
	virtual void OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform, EScriptableToolGizmoStateChangeType ChangeType) override;


	UPROPERTY()
	TObjectPtr<ASkeletalMeshCutterActor> OutputActor = nullptr;
	

private:
	UPROPERTY()
	TObjectPtr<USkeletalMeshCutterPropertySet> Settings = nullptr;

	void SaveDuplicate();

	TArray<ECutterShapeType> LastCutterArray;

	void HideAllGizmos();
	
};

UCLASS()
class HANDYMAN_API USkeletalMeshCutterPropertySet : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	
	USkeletalMeshCutterPropertySet();

	/*UFUNCTION(CallInEditor, BlueprintCallable, Category="Parameters")
	void InitializeMesh();*/
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters")
	FSkeletalMeshAssetData MeshData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters")
	TArray<ECutterShapeType> Cutters;
	
	
};

