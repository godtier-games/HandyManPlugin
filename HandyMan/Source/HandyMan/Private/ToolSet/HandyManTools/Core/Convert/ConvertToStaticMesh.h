// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TargetInterfaces/MeshTargetInterfaceTypes.h"
#include "ToolSet/Core/HandyManToolBuilder.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "ConvertToStaticMesh.generated.h"

UCLASS()
class HANDYMAN_API UConvertToStaticMeshBuilder : public UHandyManToolBuilder
{
	GENERATED_BODY()

	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;

};


/**
 * 
 */
UCLASS()
class HANDYMAN_API UConvertToStaticMesh : public UHandyManSingleClickTool
{
	GENERATED_BODY()

public:
	
	UConvertToStaticMesh();

	virtual void OnHitByClick_Implementation(FInputDeviceRay ClickPos,
	                                                 const FScriptableToolModifierStates& Modifiers) override;
	
	virtual UBaseScriptableToolBuilder* GetNewCustomToolBuilderInstance(UObject* Outer) override;

	virtual void Setup() override;

	virtual void Shutdown(EToolShutdownType ShutdownType) override;


	void HandleAccept();
	void HandleCancel();

	virtual bool CanAccept() const override{return Super::CanAccept() && Inputs.Num() > 0;};
	virtual bool HasCancel() const override{return true;};
	virtual bool HasAccept() const override {return true;};


protected:

	/** Traces the actor under cursor */
	virtual bool Trace(FHitResult& OutHit, const FInputDeviceRay& DevicePos) override;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> SelectedActors;

public:

	void InitializeInputs(TArray<TWeakObjectPtr<UPrimitiveComponent>>&& InInputs)
	{
		Inputs = MoveTemp(InInputs);
	}

	void SetTargetLOD(EMeshLODIdentifier LOD)
	{
		TargetLOD = LOD;
	}

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UPrimitiveComponent>> Inputs;

	EMeshLODIdentifier TargetLOD = EMeshLODIdentifier::Default;
};
