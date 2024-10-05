// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IndexTypes.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "ToolSet/HandyManBaseClasses/HandyManModularTool.h"
#include "ToolSet/PropertySet/HandyManToolPropertySet.h"
#include "ToolSet/ToolBuilders/HandyManToolWithTargetBuilder.h"
#include "HandyManPipeTool.generated.h"

class AHandyManPipeActor;
/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManPipeToolProperties : public UHandyManToolPropertySet
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	TObjectPtr<UMaterialInterface> PipeMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Options")
	int32 SampleSize = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	bool bProjectPointsToSurface = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	FVector2D StartEndRadius = FVector2D(1.0f, 1.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	int32 ShapeSegments = 8;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	float ShapeRadius = 4.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	float RotationAngleDeg = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options", meta = (DisplayName = "PolyGroup Mode"))
	EGeometryScriptPrimitivePolygroupMode PolygroupMode = EGeometryScriptPrimitivePolygroupMode::PerFace;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	bool bFlipOrientation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	EGeometryScriptPrimitiveUVMode UVMode = EGeometryScriptPrimitiveUVMode::Uniform;
};

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManPipeTool : public UHandyManModularTool
{
	GENERATED_BODY()

public:

	void SetTargetActors(const TArray<AActor*>& InActors); // TODO : Add this to an interface for easier access later when building new tools

	UHandyManPipeTool();

	void SpawnOutputActorInstance(const UHandyManPipeToolProperties* InSettings);

	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	
	///~ Hover Behavior
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta=(DisplayName="Can Hover"))
	FInputRayHit TestCanHoverFunc(const FInputDeviceRay& PressPos, const FScriptableToolModifierStates& Modifiers);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Begin Hover"))
	void OnBeginHover(const FInputDeviceRay& DevicePos, const FScriptableToolModifierStates& Modifiers);

	///~ IHoverBehaviorTarget API
	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Update Hover"))
	bool OnUpdateHover(const FInputDeviceRay& DevicePos, const FScriptableToolModifierStates& Modifiers);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="On End Hover"))
	void OnEndHover();

	
	///~ Click Drag Behavior
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta=(DisplayName="Can Mouse Drag"))
	FInputRayHit CanClickDrag(const FInputDeviceRay& PressPos, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button);
	
	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Update Mouse Drag"))
	void OnClickDrag(const FInputDeviceRay& DragPos, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button = EScriptableToolMouseButton::LeftButton);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Start Mouse Drag"))
	void OnDragBegin(const FInputDeviceRay& StartPosition, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="On End Mouse Drag"))
	void OnDragEnd(const FInputDeviceRay& EndPosition, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button);
	
	
	///~ Single Click Behavior
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta=(DisplayName="Can Click"))
	FInputRayHit CanClickFunc(const FInputDeviceRay& PressPos, const EScriptableToolMouseButton& Button);

	
	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Hit By Click"))
	void OnHitByClickFunc(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& MouseButton);

	UFUNCTION(BlueprintCallable)
	bool MouseBehaviorModiferCheckFunc(const FInputDeviceState& InputDeviceState);


	///~ Mouse Wheel Behavior
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta=(DisplayName="Can Use Mouse Wheel"))
	FInputRayHit CanUseMouseWheel(const FInputDeviceRay& PressPos);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Mouse Wheel Up"))
	void OnMouseWheelUp(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="On Mouse Wheel Down"))
	void OnMouseWheelDown(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers);


	///~ Gizmo Behavior
	virtual void OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform, EScriptableToolGizmoStateChangeType ChangeType) override;
	virtual void OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform) override;


protected:

	UPROPERTY()
	TObjectPtr<UHandyManPipeToolProperties> Settings;

	UPROPERTY()
	TMap<FName, AHandyManPipeActor*> OutputActorMap;

	UPROPERTY()
	TArray<AActor*> TargetActors;

	UPROPERTY()
	TArray<AHandyManPipeActor*> SelectionArray;
	

	
};


/**
 * Base Tool Builder for tools that operate on a selection of Spline Components
 */
UCLASS(Transient)
class HANDYMAN_API UHandyManPipeToolBuilder : public UHandyManToolWithTargetBuilder
{
	GENERATED_BODY()

public:
	UHandyManPipeToolBuilder(const FObjectInitializer& ObjectInitializer);
	/** @return true if spline component sources can be found in the active selection */
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;

	virtual void SetupTool(const FToolBuilderState& SceneState, UInteractiveTool* Tool) const override;

	// @return the min and max (inclusive) number of splines allowed in the selection for the tool to be built. A value of -1 can be used to indicate there is no maximum.
	virtual UE::Geometry::FIndex2i GetSupportedSplineCountRange() const
	{
		return UE::Geometry::FIndex2i(1, -1);
	}
};

