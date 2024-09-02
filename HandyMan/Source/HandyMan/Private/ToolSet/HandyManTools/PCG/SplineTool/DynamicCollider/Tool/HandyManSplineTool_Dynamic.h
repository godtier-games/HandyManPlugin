// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveToolChange.h"
#include "TransactionUtil.h"
#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "Components/SplineComponent.h"
#include "ToolSet/Core/HandyManToolBuilder.h"
#include "ToolSet/DataTypes/HandyManDataTypes.h"
#include "ToolSet/HandyManBaseClasses/HandyManInteractiveTool.h"
#include "ToolSet/Utils/SplineUtils.h"
#include "HandyManSplineTool_Dynamic.generated.h"

class UDynamicSplineToolProperties;
class APCG_DynamicSplineActor;
class USplineComponent;
class USingleClickOrDragInputBehavior;
class UConstructionPlaneMechanic;
class APreviewGeometryActor;

namespace ESplinePointType
{
	enum Type : int;
}

/**
 * 
 */
UCLASS()
class HANDYMAN_API USplineTool_Dynamic : public UHandyManInteractiveTool, public IClickBehaviorTarget
	, public IClickDragBehaviorTarget
{
	GENERATED_BODY()
public:

	USplineTool_Dynamic();

	virtual UBaseScriptableToolBuilder* GetNewCustomToolBuilderInstance(UObject* Outer) override;

	void SpawnActorInstance(const UDynamicSplineToolProperties* CurrentSettings);
	virtual void SetSelectedActor(AActor* Actor);

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
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos) override;
	virtual void OnClicked(const FInputDeviceRay& ClickPos) override;

	// IClickDragBehaviorTarget
	virtual FInputRayHit CanBeginClickDragSequence(const FInputDeviceRay& PressPos) override;
	virtual void OnClickPress(const FInputDeviceRay& PressPos) override;
	virtual void OnClickDrag(const FInputDeviceRay& DragPos) override;
	virtual void OnClickRelease(const FInputDeviceRay& ReleasePos) override;
	virtual void OnTerminateDragSequence() override;
	
	EDrawSplineDrawMode_HandyMan DrawMode;
protected:

	
	UPROPERTY()
	TObjectPtr<UDynamicSplineToolProperties> Settings = nullptr;

	UPROPERTY()
	TObjectPtr<USingleClickOrDragInputBehavior> ClickOrDragBehavior = nullptr;

	UPROPERTY()
	TObjectPtr<UConstructionPlaneMechanic> PlaneMechanic = nullptr;

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
	void SetSplinePointsOnTargetActor();

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
	TObjectPtr<APCG_DynamicSplineActor> TargetSplineActor;

	

public:
	// Helper class for making undo/redo transactions, to avoid friending all the variations.
	class FDynamicSplineToolChange : public FSplineToolChange
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
class HANDYMAN_API UDynamicSplineToolBuilder : public UHandyManToolBuilder
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
class HANDYMAN_API UDynamicSplineToolProperties : public UInteractiveToolPropertySet
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

	UPROPERTY(EditAnywhere, Category = "Geometry Settings", meta = (TransientToolProperty))
	TEnumAsByte<ESplinePointType::Type> SplineType = ESplinePointType::Type::Linear;
	
	UPROPERTY(EditAnywhere, Category = "Geometry Settings", meta = (TransientToolProperty, UIMin = 0, UIMax = 100))
	float DistanceOffset = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta=(GetOptions="HandyMan.HandyManStatics.GetCollisionProfileNames"))
	FName CollisionProfileName = FName(TEXT("BlockAll"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry Settings")
	FVector2D ColliderAdditiveScale = FVector2D(1.0f, 1.0f);

	/**
	 * Tags to apply to the created actors. Can be used for filtering in the world outliner.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings")
	TArray<FName> ActorTags;
	
	UPROPERTY(EditAnywhere, Category = "Geometry Settings", meta = (TransientToolProperty))
	bool bClosedSpline = false;

	//UPROPERTY(EditAnywhere, Category = "Geometry Settings", meta = (TransientToolProperty))
	bool bAimMeshAtNextPoint = false;

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
