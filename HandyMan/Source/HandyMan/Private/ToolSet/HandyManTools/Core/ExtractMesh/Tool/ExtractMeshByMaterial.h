// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "ToolSet/HandyManTools/Core/ExtractMesh/Data/ExtractMeshDataTypes.h"
#include "ToolSet/PropertySet/HandyManToolPropertySet.h"
#include "ExtractMeshByMaterial.generated.h"

class AExtractMeshProxyActor;

UCLASS()
class HANDYMAN_API UExtractMeshByMaterialPropertySet : public UHandyManToolPropertySet
{
	GENERATED_BODY()

public:
	
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Parameters")
	TObjectPtr<USkeletalMesh> InputMesh;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bSaveMeshesAsStaticMesh = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Parameters")
	bool bMergeMeshes = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Parameters", meta=(EditCondition= "!bMergeMeshes", EditConditionHides))
	float MeshDisplayOffset = 65.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters", NoClear)
	TArray<FExtractedMeshInfo> MeshInfo;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Parameters", NoClear)
	FString FolderName = TEXT("GENERATED");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Parameters", NoClear, meta=(EditCondition= "bMergeMeshes", EditConditionHides))
	FString MergedAssetName = TEXT("MERGED");
	
	UPROPERTY()
	TArray<FExtractedMeshInfo> MeshInfoCache;
	
	
};

/**
 * 
 */
UCLASS()
class HANDYMAN_API UExtractMeshByMaterial : public UHandyManSingleClickTool
{
	GENERATED_BODY()

public:

	UExtractMeshByMaterial();

	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual bool CanAccept() const override;

	void SpawnOutputActorInstance(const UExtractMeshByMaterialPropertySet* InSettings, const FTransform& SpawnTransform);

	virtual FInputRayHit TestIfHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;
	virtual void OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;
	
	virtual void OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform) override;
	virtual void OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform, EScriptableToolGizmoStateChangeType ChangeType) override;
	
	UPROPERTY()
	TObjectPtr<AExtractMeshProxyActor> OutputActor = nullptr;
	

private:
	UPROPERTY()
	TObjectPtr<UExtractMeshByMaterialPropertySet> Settings = nullptr;

	void SaveDuplicate();

};
