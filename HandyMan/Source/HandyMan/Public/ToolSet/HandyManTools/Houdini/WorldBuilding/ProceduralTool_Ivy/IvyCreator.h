// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HoudiniEngineEditor/Private/HoudiniEngineEditor.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "IvyCreator.generated.h"

class UHoudiniPublicAPIAssetWrapper;
/**
 * 
 */
UCLASS()
class HANDYMAN_API UIvyCreator : public UHandyManSingleClickTool
{
	GENERATED_BODY()

public:
	UIvyCreator();

	virtual void Setup() override;

	// Click
	//virtual FInputRayHit TestIfHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;
	virtual void OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;

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
	void HighlightSelectedActor(const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection,
	                            const FHitResult& HitResult);

	void HandleAccept();

private:
	
	UPROPERTY()
	class UIvyCreator_PropertySet* PropertySet;

	UPROPERTY()
	TMap<UObject*, UHoudiniPublicAPIAssetWrapper*> SelectedActors;

	void HighlightActors(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers, bool bShouldEditSelection = false);

	UHoudiniPublicAPIAssetWrapper* SpawnHDAInstance();

};

UCLASS()
class HANDYMAN_API UIvyCreator_PropertySet : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	UIvyCreator_PropertySet();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ivy Creator | Filtering", meta = (Tooltip = "Select an array of actors that the tool will run on"))
	TArray<TSubclassOf<AActor>> Filter;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ivy Creator | Filtering", meta = (Tooltip = "Use a custom trace channel to further filter what should or should not interact with the tool."))
	bool bUseCustomTraceChannel = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ivy Creator | Filtering", meta = (Tooltip = "Custom Trace Channel"), meta=(EditCondition="bUseCustomTraceChannel", EditConditionHides))
	TEnumAsByte<ECollisionChannel> CustomTraceChannel = ECC_Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ivy Creator | Filtering", meta = (Tooltip = "This will negate the filter array so that no actors of this type will react to the tool."))
	bool bExcludeFilter = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ivy Creator | Baking", meta = (Tooltip = "Do you want to keep the procedural HDA's in the level so that you can manually work with them"))
	bool bKeepProceduralAssets = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ivy Creator | Baking", meta = (Tooltip = "Do you want to keep the procedural HDA's in the level so that you can manually work with them"))
	bool bReplaceExistingBake = false;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ivy Creator | Baking", meta = (Tooltip = "Will This Be A Static Mesh Or A Blueprint Actor?"))
	EHoudiniEngineBakeOption BakeType = EHoudiniEngineBakeOption::ToActor;
#endif
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ivy Creator | Baking", meta = (Tooltip = "The File Path You Want To Store The Baked Actor"), meta=(RelativeToGameContentDir="true"))
	FFilePath AssetSaveLocation;

	
};
