// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTools/ScriptableSingleClickTool.h"
#include "HandyManSingleClickTool.generated.h"

class UHoudiniAsset;
class UHoudiniPublicAPI;
/**
 * 
 */
UCLASS(Abstract)
class HANDYMAN_API UHandyManSingleClickTool : public UScriptableSingleClickTool
{
	GENERATED_BODY()

public:

	/** Returns the Houdini Public API instance. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Houdini Engine"), Category = "Houdini Engine")
	const UHoudiniPublicAPI* GetHoudiniAPI() const {return HoudiniPublicAPI;}
	
	UHoudiniPublicAPI* GetMutableAPI() const {return HoudiniPublicAPI;}

	UHoudiniAsset* GetHoudiniDigitalAsset() const {return HDA;}

	UPROPERTY(EditAnywhere, Category = "Houdini")
	FString HDAFileName;
	
protected:
	UFUNCTION(BlueprintCallable, Category = "Houdini")
	void CacheHoudiniAPI();

	UFUNCTION(BlueprintCallable, Category = "Houdini")
	void InitHoudiniDigitalAsset();

	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	UPROPERTY(EditDefaultsOnly, Category = "Houdini")
	TObjectPtr<UHoudiniAsset> HDA;
	
private:
	UPROPERTY()
	UHoudiniPublicAPI* HoudiniPublicAPI;

	


	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);
};
