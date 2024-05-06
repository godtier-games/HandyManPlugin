// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "HandyManSubsystem.generated.h"

class UHoudiniAssetWrapper;
class UHoudiniAsset;
class UHoudiniPublicAPI;
/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	/** Returns the Houdini Public API instance. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Houdini Engine"), Category = "Houdini Engine")
	const UHoudiniPublicAPI* GetHoudiniAPI() const {return HoudiniPublicAPI;}
	
	UHoudiniPublicAPI* GetMutableHoudiniAPI() const {return HoudiniPublicAPI;}

	UHoudiniAsset* GetHoudiniDigitalAsset(const FName& ToolName) const;
	
	UFUNCTION(BlueprintCallable, Category = "Houdini")
	void InitializeHoudiniApi();

	UFUNCTION(BlueprintCallable, Category = "Houdini")
	void CleanUp();
private:
	UPROPERTY()
	UHoudiniPublicAPI* HoudiniPublicAPI;
	
};
