// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveToolQueryInterfaces.h"
#include "ToolSet/HandyManBaseClasses/HandyManClickDragTool.h"
#include "ToolSet/ToolBuilders/HandyManToolWithTargetBuilder.h"
#include "HandyManMeshSurfaceTool.generated.h"


// This is a temporary interface to provide stylus pressure, currently necessary due to limitations
// in the stylus plugin architecture. Should be removed once InputState/InputBehavior can support stylus events
class IHandyManStylusStateProviderAPI
{
public:
	virtual float GetCurrentPressure() const = 0;
};

UCLASS()
class HANDYMAN_API  UHandyManMeshSurfaceToolBuilder : public UHandyManToolWithTargetBuilder
{
	GENERATED_BODY()

public:

	IHandyManStylusStateProviderAPI* StylusAPI = nullptr;

	UHandyManMeshSurfaceToolBuilder(const FObjectInitializer& ObjectInitializer);

	
	virtual void SetupTool(const FToolBuilderState& SceneState, UInteractiveTool* Tool) const override;

	//virtual void OnSetupTool_Implementation(UScriptableInteractiveTool* Tool, const TArray<AActor*>& SelectedActors, const TArray<UActorComponent*>& SelectedComponents) const override;

	
};



/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManMeshSurfaceTool : public UHandyManClickDragTool, public IInteractiveToolCameraFocusAPI
{
	GENERATED_BODY()
	
public:

	virtual void SetStylusAPI(IHandyManStylusStateProviderAPI* StylusAPI);
	
	virtual bool HitTest(const FRay& Ray, FHitResult& OutHit);
	
	virtual bool SupportsWorldSpaceFocusBox() override;
	virtual FBox GetWorldSpaceFocusBox() override;
	virtual bool SupportsWorldSpaceFocusPoint() override;

	virtual float GetCurrentDevicePressure() const;

	/**
	 * @param WorldRay 3D Ray that should be used to find the focus point, generally ray under cursor
	 * @param PointOut computed Focus Point
	 * @return true if a Focus Point was found, can return false if (eg) the ray missed the target objects
	 */
	virtual bool GetWorldSpaceFocusPoint(const FRay& WorldRay, FVector& PointOut) override;


	/**
	 * @return true if all ToolTargets of this tool are still valid
	 */
	virtual bool AreAllTargetsValid() const
	{
		return Targets.Num() == 1  ? Targets[0]->IsValid() : false;
	}


	virtual bool CanAccept() const override
	{
		return AreAllTargetsValid();
	}

	/**
	 * This function is called when the user begins a click-drag-release interaction
	 */
	virtual void OnBeginDrag(const FRay& Ray);

	/**
	 * This function is called each frame that the user is in a click-drag-release interaction
	 */
	virtual void OnUpdateDrag(const FRay& Ray);

	/**
	 * This function is called when the user releases the button driving a click-drag-release interaction
	 */
	virtual void OnEndDrag(const FRay& Ray);

	/**
	 * This function is called when the user's drag is cancelled, for example due to the whole tool being shut down.
	 */
	virtual void OnCancelDrag() {}


	virtual FInputRayHit OnHoverHitTest_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers) override;
	
	virtual FInputRayHit TestIfCanBeginClickDrag_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;

	virtual void OnDragBegin_Implementation(FInputDeviceRay StartPosition, const FScriptableToolModifierStates& Modifiers) override;

	virtual void OnDragUpdatePosition_Implementation(FInputDeviceRay NewPosition, const FScriptableToolModifierStates& Modifiers) override;

	virtual void OnDragEnd_Implementation(FInputDeviceRay EndPosition, const FScriptableToolModifierStates& Modifiers) override;

	virtual void OnDragSequenceCancelled_Implementation() override;


	
protected:
	FRay LastWorldRay;

	IHandyManStylusStateProviderAPI* StylusAPI = nullptr;
};
