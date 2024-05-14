// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManTools/Houdini/WorldBuilding/SplineTool/DrawSpline.h"
#include "SplineFenceTool.generated.h"


class APCG_SplineActor;
/**
 * 
 */
UCLASS()
class HANDYMAN_API USplineTool : public UHandyManInteractiveTool, public IClickBehaviorTarget
	, public IClickDragBehaviorTarget
{
	GENERATED_BODY()
public:

	virtual UBaseScriptableToolBuilder* GetNewCustomToolBuilderInstance(UObject* Outer) override;

	void SpawnActorInstance();
	virtual void SetSelectedActor(AActor* Actor);

	virtual void SetWorld(UWorld* World);
	virtual UWorld* GetTargetWorld() { return TargetWorld.Get(); }

	// UInteractiveTool
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual void OnTick(float DeltaTime) override;
	virtual bool HasCancel() const override { return true; }
	virtual bool HasAccept() const override { return true; }
	virtual bool CanAccept() const override;
	virtual void Render(IToolsContextRenderAPI* RenderAPI) override;
	virtual void OnPropertyModified(UObject* PropertySet, FProperty* Property) override;

	// IClickBehaviorTarget
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos);
	virtual void OnClicked(const FInputDeviceRay& ClickPos);

	// IClickDragBehaviorTarget
	virtual FInputRayHit CanBeginClickDragSequence(const FInputDeviceRay& PressPos);
	virtual void OnClickPress(const FInputDeviceRay& PressPos);
	virtual void OnClickDrag(const FInputDeviceRay& DragPos);
	virtual void OnClickRelease(const FInputDeviceRay& ReleasePos);
	virtual void OnTerminateDragSequence();

	EHoudiniPublicAPICurveType CurveType;
	
	EDrawSplineDrawMode_HandyMan DrawMode;

	EHandyManToolName ToolIdentifier;

protected:

	
	UPROPERTY()
	TObjectPtr<USplineToolProperties> Settings = nullptr;

	UPROPERTY()
	TObjectPtr<USingleClickOrDragInputBehavior> ClickOrDragBehavior = nullptr;

	UPROPERTY()
	TObjectPtr<UConstructionPlaneMechanic> PlaneMechanic = nullptr;

	TWeakObjectPtr<UWorld> TargetWorld = nullptr;

	// This is only used to initialize TargetActor in the settings object
	TWeakObjectPtr<AActor> SelectedActor = nullptr;

	// PreviewRootActor either holds WorkingSpline inside it directly, or has some preview actor
	// attached to it (so that the preview actor is hidden from outliner, like APreviewGeometryActor is).
	UPROPERTY()
	TObjectPtr<APreviewGeometryActor> PreviewRootActor = nullptr;

	// The preview actor may be a duplicate of some target blueprint actor so that we can see the
	// effects of the drawn spline immediately
	UPROPERTY()
	TObjectPtr<AActor> PreviewActor = nullptr;

	// Used for recapturing the spline when rerunning construction scripts
	int32 SplineRecaptureIndex = 0;

	// This is the spline we add points to. It points to a component nested somewhere under 
	// PreviewRootActor.
	TWeakObjectPtr<USplineComponent> WorkingSpline = nullptr;

	bool bDrawTangentForLastPoint = false;
	bool bFreeDrawPlacedPreviewPoint = false;
	int32 FreeDrawStrokeStartIndex = 0;

	bool Raycast(const FRay& WorldRay, FVector3d& HitLocationOut, FVector3d& HitNormalOut, double& HitTOut);
	void AddSplinePoint(const FVector3d& HitLocation, const FVector3d& UpVector);
	FVector3d GetUpVectorToUse(const FVector3d& HitLocation, const FVector3d& HitNormal, int32 NumSplinePointsBeforehand);

	void TransitionOutputMode();
	void GenerateAsset();
	
	// Used to restore visibility of previous actor when switching to a different one
	UPROPERTY()
	TObjectPtr<AActor> PreviousTargetActor = nullptr;
	bool PreviousTargetActorVisibility = true;
	// Used to restore visibility of previous spline when switching to a different one
	TWeakObjectPtr<USplineComponent> HiddenSpline = nullptr;
	bool bPreviousSplineVisibility = true;

	bool bNeedToRerunConstructionScript = false;

	FViewCameraState CameraState;

private:
	UE::TransactionUtil::FLongTransactionTracker LongTransactions;

public:
	UPROPERTY()
	TObjectPtr<APCG_SplineActor> TargetSplineActor;

	

public:
	// Helper class for making undo/redo transactions, to avoid friending all the variations.
	class FSplineToolChange : public FToolCommandChange
	{
	public:
		// These pass the working spline to the overloads below
		virtual void Apply(UObject* Object) override;
		virtual void Revert(UObject* Object) override;

	protected:
		virtual void Apply(USplineComponent& Spline) = 0;
		virtual void Revert(USplineComponent& Spline) = 0;
	};
};



#pragma region BUILDER

UCLASS(Transient)
class HANDYMAN_API USplineToolBuilder : public UHandyManToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;

	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};

#pragma endregion


#pragma region SETTINGS
class UHoudiniPublicAPIAssetWrapper;
class USingleClickOrDragInputBehavior;
class UConstructionPlaneMechanic;
class USplineComponent;


UCLASS()
class HANDYMAN_API USplineToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	//~ This is TransientToolProperty because while it's useful to toggle after drawing a spline,
	//~ it's confusing when you first start the tool and are drawing a spline.
	/** 
	 * Determines whether the created spline is a loop. This can be toggled using "Closed Loop" in
	 * the detail panel after spline creation.
	 */
	//UPROPERTY(EditAnywhere, Category = Spline, meta = (TransientToolProperty))
	bool bLoop = false;

	/**
	 * Determines how the resulting spline is emitted on tool accept.
	 */
	//UPROPERTY(EditAnywhere, Category = Spline)
	EDrawSplineOutputMode_HandyMan OutputMode = EDrawSplineOutputMode_HandyMan::EmptyActor;

	UPROPERTY(EditAnywhere, Category = "Geometry Settings", meta = (TransientToolProperty))
	TSoftObjectPtr<UStaticMesh> InputGeometry = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Geometry Settings", meta = (TransientToolProperty, UIMin = -1000, UIMax = 1000))
	float MeshDistance = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Geometry Settings", meta = (TransientToolProperty, UIMin = 0.1, UIMax = 10))
	FVector2D MeshHeightRange = FVector2D(1.0f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Geometry Settings", meta = (TransientToolProperty))
	bool bEnableRandomRotation = false;
	
	UPROPERTY(EditAnywhere, Category = "Geometry Settings", meta = (EditCondition = "bEnableRandomRotation", EditConditionHides, TransientToolProperty, UIMin = -180, UIMax = 180))
	FRotator MinRandomRotation = FRotator();
	
	UPROPERTY(EditAnywhere, Category = "Geometry Settings", meta = (EditCondition = "bEnableRandomRotation", EditConditionHides, TransientToolProperty, UIMin = -180, UIMax = 180))
	FRotator MaxRandomRotation = FRotator();

	UPROPERTY(EditAnywhere, Category = "Geometry Settings", meta = (EditCondition = "bEnableRandomRotation", EditConditionHides, TransientToolProperty))
	int32 RandomizedSeed = 1729.0f;
	
	/**
	 * Actor to attach to when Output Mode is "Existing Actor"
	 */
	/*UPROPERTY(EditAnywhere, Category = Spline, meta = (TransientToolProperty, 
		EditCondition = "OutputMode == EDrawSplineOutputMode_HandyMan::ExistingActor", EditConditionHides))*/
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	/**
	 * Blueprint to create when Output Mode is "Create Blueprint"
	 */
	/*UPROPERTY(EditAnywhere, Category = Spline, meta = (
		EditCondition = "OutputMode == EDrawSplineOutputMode_HandyMan::CreateBlueprint", EditConditionHides))*/
	TWeakObjectPtr<UBlueprint> BlueprintToCreate;

	/**
	 * When attaching to an existing actor or creating a blueprint, controls whether the drawn
	 * spline replaces one of the target's existing components or gets added as a new one (if
	 * the index is out of bounds).
	 */
	/*
	UPROPERTY(EditAnywhere, Category = Spline, meta = (ClampMin = -1, UIMax = 10, EditConditionHides,
		EditCondition = "OutputMode == EDrawSplineOutputMode_HandyMan::ExistingActor || OutputMode == EDrawSplineOutputMode_HandyMan::CreateBlueprint"))
	*/
	int32 ExistingSplineIndexToReplace = 0;

	/**
	 * How the spline is drawn in the tool.
	 */
	//UPROPERTY(EditAnywhere, Category = Spline)
	EDrawSplineDrawMode_HandyMan DrawMode = EDrawSplineDrawMode_HandyMan::None;
	
	/** Point spacing when Draw Mode is "Free Draw" */
	/*UPROPERTY(EditAnywhere, Category = Spline, meta = (ClampMin = 0, UIMax = 1000,
		EditCondition = "DrawMode == EDrawSplineDrawMode_HandyMan::FreeDraw", EditConditionHides))*/
	double MinPointSpacing = 200;

	/** How far to offset spline points from the clicked surface, along the surface normal */
	//UPROPERTY(EditAnywhere, Category = Spline, meta = (UIMin = 0, UIMax = 100))
	double ClickOffset = 0;

	/** How to choose the direction to offset points from the clicked surface */
	//UPROPERTY(EditAnywhere, Category = Spline, meta = (EditCondition = "ClickOffset > 0", EditConditionHides))
	ESplineOffsetMethod_HandyMan OffsetMethod = ESplineOffsetMethod_HandyMan::HitNormal;

	/** Manually-specified click offset direction. Note: Will be normalized. If it is a zero vector, a default Up vector will be used instead. */
	//UPROPERTY(EditAnywhere, Category = Spline, meta = (EditCondition = "ClickOffset > 0 && OffsetMethod == ESplineOffsetMethod_HandyMan::Custom", EditConditionHides))
	FVector OffsetDirection = FVector::UpVector;

	/**
	 * When nonzero, allows a visualization of the rotation of the spline. Can be controlled
	 * in the detail panel after creation via "Scale Visualization Width" property.
	 */
	UPROPERTY(EditAnywhere, Category = Spline, AdvancedDisplay, meta = (ClampMin = 0, UIMax = 100))
	double FrameVisualizationWidth = 0;

	/**
	 * How the spline rotation is set. It is suggested to use a nonzero FrameVisualizationWidth to
	 * see the effects.
	 */
	//UPROPERTY(EditAnywhere, Category = Spline, AdvancedDisplay)
	EDrawSplineUpVectorMode_HandyMan UpVectorMode = EDrawSplineUpVectorMode_HandyMan::UseHitNormal;

	/**
	 * When modifying existing actors, whether the result should be previewed using a copy
	 * of that actor (rather than just drawing the spline by itself). Could be toggled off
	 * if something about duplicating the given actor is problematic.
	 */
	/*UPROPERTY(EditAnywhere, Category = Spline, AdvancedDisplay, meta=(
		EditCondition = "OutputMode != EDrawSplineOutputMode_HandyMan::EmptyActor"))*/
	bool bPreviewUsingActorCopy = true;

	/** Whether to place spline points on the surface of objects in the world */
	UPROPERTY(EditAnywhere, Category = RaycastTargets, meta = (DisplayName = "World Objects"))
	bool bHitWorld = true;

	/** Whether to place spline points on a custom, user-adjustable plane */
	UPROPERTY(EditAnywhere, Category = RaycastTargets, meta = (DisplayName = "Custom Plane"))
	bool bHitCustomPlane = false;

	/** Whether to place spline points on a plane through the origin aligned with the Z axis in perspective views, or facing the camera in othographic views */
	UPROPERTY(EditAnywhere, Category = RaycastTargets, meta = (DisplayName = "Ground Planes"))
	bool bHitGroundPlanes = true;

	/**
	 * If modifying a blueprint actor, whether to run the construction script while dragging
	 * or only at the end of a drag. Can be toggled off for expensive construction scripts.
	 */
	/*UPROPERTY(EditAnywhere, Category = Spline, AdvancedDisplay, meta = (
		EditCondition = "OutputMode != EDrawSplineOutputMode_HandyMan::EmptyActor"))*/
	bool bRerunConstructionScriptOnDrag = true;
};


/**
 * 
 */
#pragma endregion
