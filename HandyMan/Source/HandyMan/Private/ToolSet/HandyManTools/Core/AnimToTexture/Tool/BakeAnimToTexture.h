// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimToTextureDataAsset.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "ToolSet/HandyManTools/Core/AnimToTexture/Data/AnimToTextureDataTypes.h"
#include "ToolSet/PropertySet/HandyManToolPropertySet.h"
#include "BakeAnimToTexture.generated.h"

class AAnimToTextureProxyActor;

UCLASS()
class HANDYMAN_API UBakeAnimToTexturePropertySet : public UHandyManToolPropertySet
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> Source;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BakeAnimToTexture")
	FString FolderName = TEXT("VAT");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
	bool bEnforcePowerOfTwo = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
	EAnimToTexturePrecision Precision = EAnimToTexturePrecision::EightBits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAnimTextureResolution TextureResolution = EAnimTextureResolution::Size_8192;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UAnimSequence>> AnimationsToBake;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> Target;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimToTextureDataAsset> Data;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> BonePositionTexture;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> BoneRotationTexture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> BoneWeightTexture;
	
	
	bool HasDataToBake() const
	{
		return Target != nullptr && Data != nullptr && BonePositionTexture != nullptr && BoneRotationTexture != nullptr && BoneWeightTexture != nullptr && AnimationsToBake.Num() > 0;
	}
};

/**
 * 
 */
UCLASS()
class HANDYMAN_API UBakeAnimToTexture : public UHandyManSingleClickTool
{
	GENERATED_BODY()

public:
	
	UBakeAnimToTexture();

	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual bool CanAccept() const override;

	void SpawnOutputActorInstance(const UBakeAnimToTexturePropertySet* InSettings, const FTransform& SpawnTransform);

	virtual FInputRayHit TestIfHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;
	virtual void OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;
	
	virtual void OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform) override;
	virtual void OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform, EScriptableToolGizmoStateChangeType ChangeType) override;

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateMaterialInstanceFromDataAsset(const UAnimToTextureDataAsset* DataAsset, UMaterialInstanceConstant* MaterialInstance,
		const EMaterialParameterAssociation MaterialParameterAssociation = LayerParameter);

	UFUNCTION(BlueprintImplementableEvent)
	void AnimationToTexture(UAnimToTextureDataAsset* DataAsset);


	
	UPROPERTY()
	AAnimToTextureProxyActor* OutputActor = nullptr;

	void Initialize();
	

private:
	UPROPERTY()
	UBakeAnimToTexturePropertySet* Settings = nullptr;

	
	void CreateStaticMesh();
	void CreateDataAsset();
	void CreateTextures();

	
	
};
