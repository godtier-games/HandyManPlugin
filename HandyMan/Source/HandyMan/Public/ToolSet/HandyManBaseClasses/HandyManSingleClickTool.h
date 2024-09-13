// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ScriptableInteractiveTool.h"
#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "Interfaces/HandyManToolInterface.h"
#include "ToolSet/Core/HandyManSubsystem.h"
#include "HandyManSingleClickTool.generated.h"


class UMouseHoverBehavior;
class USingleClickInputBehavior;
/*class UHoudiniAsset;
class UHoudiniPublicAPI;*/
/**
 * 
 */
UCLASS(Abstract)
class HANDYMAN_API UHandyManSingleClickTool :
public UScriptableInteractiveTool,
public IClickBehaviorTarget,
public IHoverBehaviorTarget,
public IHandyManToolInterface
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostLoad() override;
#endif

#if WITH_EDITORONLY_DATA
	FText LastKnownName;
#endif
	
	const UHandyManSubsystem* GetHandyManAPI_Safe() const {return HandyManAPI;}
	UHandyManSubsystem* GetHandyManAPI() const {return HandyManAPI;}

	virtual UBaseScriptableToolBuilder* GetHandyManToolBuilderInstance(UObject* Outer) override;
	

	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual bool Trace(FHitResult& OutHit, const FInputDeviceRay& DevicePos) override {return false;}
	
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);

private:
	UPROPERTY()
	UHandyManSubsystem* HandyManAPI;

	public:
	/**
	 * Enable Hover support API functions OnHoverHitTest / OnHoverBegin / OnHoverUpdate / OnHoverEnd for Mouse devices.
	 * Defaults to disabled.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SingleClick Tool Settings")
	bool bWantMouseHover = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SingleClick Tool Settings")
	bool bHitTestOnRelease = false;

	//
	// Click Support API
	//
public:
	/**
	 * TestIfHitByClick is called to allow the Tool to indicate if it would like to consume a potential click at ClickPos.
	 * The Tool can return yes/no and a "hit depth", which will be used to determine if the Tool is given the click (ie if it has the nearest depth),
	 * at which point the OnHitByClick event will fire. The default TestIfHitByClick implementation always captures the click (at depth 0).
	 * 
	 * Note that this function will be called twice for a particular hit - once on mouse-down and once on mouse-up, and if it returns no-hit on mouse-up, 
	 * the click will be "cancelled", similar to standard GUI click interactions.
	 * 
	 * @param ClickPos the position of the click, including both a 3D ray from the eye, and (optionally) a 2D mouse position
	 * @param Modifiers current modifier key/button state
	 * @return a FInputRayHit indicating a hit (true/false) and the hit depth along the hit-ray (0 for "always consume")
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Click Events")
	FInputRayHit TestIfHitByClick(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers);

	virtual FInputRayHit TestIfHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers);

	/**
	 * OnHitByClick is called when the Tool has indicated it would like to consume a click event (via TestIfHitByClick), and the click was not cancelled
	 * or consumed by a nearer object. 
	 * @param ClickPos the position of the click, including both a 3D ray from the eye, and (optionally) a 2D mouse position
	 * @param Modifiers current modifier key/button state
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Click Events")
	void OnHitByClick(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers);

	virtual void OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers);


	//
	// Hover Support API
	//
public:

	/**
	 * OnHoverHitTest is called to allow the Tool to indicate if it would like to start consuing "hover" input at HoverPos.
	 * The Tool can return yes/no and a "hit depth", which will be used to determine if the Tool is given the active hover input stream.
	 * The default OnHoverHitTest implement always captures the hover.
	 * 
	 * Once the hover is accepted, the OnHoverBegin event will fire, and then a stream of OnHoverUpdate events. If at any point
	 * the hover is no longer relevant, OnHoverUpdate should return false. OnHoverEnd will be called at that point, or if 
	 * the hover is cancelled for any reason (eg mouse goes out of window, button is pressed, etc)
	 * 
	 * @param HoverPos the current position of the cursor/device, including both a 3D ray from the eye, and (optionally) a 2D mouse position
	 * @param Modifiers current modifier key/button state
	 * @return a FInputRayHit indicating a hit (true/false) and the hit depth along the hit-ray (0 for "always consume")
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Hover Events")
	FInputRayHit OnHoverHitTest(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers);

	virtual FInputRayHit OnHoverHitTest_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers);

	/**
	 * Begin a sequence of Hover input events 
	 * @param HoverPos the current position of the cursor/device, including both a 3D ray from the eye, and (optionally) a 2D mouse position
	 * @param Modifiers current modifier key/button state
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Hover Events")
	void OnHoverBegin(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers);

	virtual void OnHoverBegin_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers);

	/**
	 * Update an active Hover sequence. OnHoverUpdate is only ever called between OnHoverBegin and OnHoverEnd
	 * @param HoverPos the current position of the cursor/device, including both a 3D ray from the eye, and (optionally) a 2D mouse position
	 * @param Modifiers current modifier key/button state
	 * @return true to continue hovering, false to stop receiving additional hover events
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Hover Events")
	UPARAM(DisplayName="Continue Hovering") bool
	OnHoverUpdate(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers);

	virtual bool OnHoverUpdate_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers);

	/**
	 * Indicate that an active captured Hover sequence has ended. The device may no longer be in the viewport, so no position can be provided.
	 * @param Modifiers current modifier key/button state
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Hover Events")
	void OnHoverEnd(const FScriptableToolModifierStates& Modifiers);

	virtual void OnHoverEnd_Implementation(const FScriptableToolModifierStates& Modifiers);

	UFUNCTION(BlueprintCallable, Category = "ScriptableTool|Input")
	UPARAM(DisplayName="In Hover State") bool InActiveHover() const;

	//
	// Modifer Buttons Support API
	//
public:

	/** @return true if the Shift key is currently held down */
	UFUNCTION(BlueprintCallable, Category="ScriptableTool|Input")
	UPARAM(DisplayName="Shift Down") bool IsShiftDown() const;

	/** @return true if the Ctrl key is currently held down */
	UFUNCTION(BlueprintCallable, Category="ScriptableTool|Input")
	UPARAM(DisplayName="Ctrl Down") bool IsCtrlDown() const;

	/** @return true if the Alt key is currently held down */
	UFUNCTION(BlueprintCallable, Category="ScriptableTool|Input")
	UPARAM(DisplayName="Alt Down") bool IsAltDown() const;

	/** @return a struct containing the current Modifier key states */
	UFUNCTION(BlueprintCallable, Category="ScriptableTool|Input")
	FScriptableToolModifierStates GetActiveModifiers();



public:

	UPROPERTY(Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	TObjectPtr<USingleClickInputBehavior> SingleClickBehavior;

	UPROPERTY(Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	TObjectPtr<UMouseHoverBehavior> MouseHoverBehavior;

	bool bInHover = false;

	bool bShiftModifier = false;
	bool bCtrlModifier = false;
	bool bAltModifier = false;

	// IClickBehaviorTarget API
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos);
	virtual void OnClicked(const FInputDeviceRay& ClickPos);
	virtual void OnUpdateModifierState(int ModifierID, bool bIsOn);

	// IHoverBehaviorTarget API
	virtual FInputRayHit BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos);
	virtual void OnBeginHover(const FInputDeviceRay& DevicePos);
	virtual bool OnUpdateHover(const FInputDeviceRay& DevicePos);
	virtual void OnEndHover();
};
