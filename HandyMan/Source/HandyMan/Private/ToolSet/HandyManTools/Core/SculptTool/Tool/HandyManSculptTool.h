// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FrameTypes.h"
#include "TransactionUtil.h"
#include "Changes/ValueWatcher.h"
#include "Components/BaseDynamicMeshComponent.h"
#include "ToolSet/HandyManTools/Core/SculptTool/DataTypes/HandyManSculptingTypes.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Operators/HandyManMeshBrushOperators.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Properties/MeshEditing/HandyManMeshEditingProperties.h"
#include "ToolSet/HandyManTools/Core/SurfacePointTool/HandyManMeshSurfaceTool.h"
#include "HandyManSculptTool.generated.h"

class UHandyManBrushStampIndicator;
class UMaterialInstanceDynamic;
class UCombinedTransformGizmo;
class UTransformProxy;
class UPreviewMesh;

#pragma region ToolSets






UCLASS()
class HANDYMAN_API UHandyManSculptBrushProperties : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category = Brush, meta = (DisplayPriority = 1))
	FHandyManBrushToolRadius BrushSize;

	/** Amount of falloff to apply (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, Category = Brush, meta = (DisplayName = "Falloff", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0", HideEditConditionToggle, EditConditionHides, EditCondition="bShowFalloff", DisplayPriority = 3))
	float BrushFalloffAmount;

	/** If false, then BrushFalloffAmount will not be shown in DetailsView panels (otherwise no effect) */
	UPROPERTY(meta = (TransientToolProperty))
	bool bShowFalloff = true;

	/** Depth of Brush into surface along view ray or surface normal, depending on the Active Brush Type */
	UPROPERTY(EditAnywhere, Category = Brush, meta = (UIMin = "-0.5", UIMax = "0.5", ClampMin = "-1.0", ClampMax = "1.0", DisplayPriority = 5, HideEditConditionToggle, EditConditionHides, EditCondition = "bShowPerBrushProps"))
	float Depth = 0;

	/** Allow the Brush to hit the back-side of the mesh */
	UPROPERTY(EditAnywhere, Category = Brush, meta = (DisplayPriority = 6))
	bool bHitBackFaces = true;

	/** Brush stamps are applied at this time interval. 0 for a single stamp, 1 for continuous stamps, 0.5 is a stamp every half-second */
	UPROPERTY(EditAnywhere, Category = Brush, meta = (DisplayName = "Flow", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0", DisplayPriority = 7, HideEditConditionToggle, EditConditionHides, EditCondition = "bShowFlowRate"))
	float FlowRate = 1.0f;

	/** Space out stamp centers at distances Spacing*BrushDiameter along the stroke (so Spacing of 1 means that stamps will be adjacent but non-overlapping). Zero spacing means continuous stamps. */
	UPROPERTY(EditAnywhere, Category = Brush, meta = (UIMin = "0.0", UIMax = "4.0", ClampMin = "0.0", ClampMax = "1000.0", DisplayPriority = 8, HideEditConditionToggle, EditConditionHides, EditCondition = "bShowSpacing"))
	float Spacing = 0.0f;

	/** Lazy brush smooths out the brush path by averaging the cursor positions */
	UPROPERTY(EditAnywhere, Category = Brush, meta = (DisplayName = "Lazy", UIMin = "0", UIMax = "1.0", ClampMin = "0", ClampMax = "1.0", DisplayPriority = 9, HideEditConditionToggle, EditConditionHides, EditCondition = "bShowLazyness"))
	float Lazyness = 0;

	/**  */
	UPROPERTY( meta = (TransientToolProperty))
	bool bShowPerBrushProps = true;

	/**  */
	UPROPERTY(meta = (TransientToolProperty))
	bool bShowLazyness = true;

	/**  */
	UPROPERTY(meta = (TransientToolProperty))
	bool bShowFlowRate = true;

	UPROPERTY(meta = (TransientToolProperty))
	bool bShowSpacing = true;
};




UCLASS()
class HANDYMAN_API UHandyManKelvinBrushProperties : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	/** Brush Fall off as fraction of brush size*/
	UPROPERTY(EditAnywhere, Category = Kelvin, meta = (UIMin = "0.0", ClampMin = "0.0"))
	float FallOffDistance = 1.f;

	/** How much the mesh resists shear */
	UPROPERTY(EditAnywhere, Category = Kelvin, meta = (UIMin = "0.0", ClampMin = "0.0"))
	float Stiffness = 1.f;

	/** How compressible the spatial region is: 1 - 2 x Poisson ratio */
	UPROPERTY(EditAnywhere, Category = Kelvin, meta = (UIMin = "0.0", UIMax = "0.1", ClampMin = "0.0", ClampMax = "1.0"))
	float Incompressiblity = 1.f;

	/** Integration steps*/
	UPROPERTY(EditAnywhere, Category = Kelvin, meta = (UIMin = "0.0", UIMax = "100", ClampMin = "0.0", ClampMax = "100"))
	int BrushSteps = 3;
};




UCLASS()
class HANDYMAN_API UHandyManWorkPlaneProperties : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	UPROPERTY( meta = (TransientToolProperty) )
	bool bPropertySetEnabled = true;

	/** Toggle whether Work Plane Positioning Gizmo is visible */
	UPROPERTY(EditAnywhere, Category = TargetPlane, meta = (HideEditConditionToggle, EditCondition = "bPropertySetEnabled == true"))
	bool bShowGizmo = true;

	UPROPERTY(EditAnywhere, Category = TargetPlane, AdvancedDisplay, meta = (HideEditConditionToggle, EditCondition = "bPropertySetEnabled == true"))
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = TargetPlane, AdvancedDisplay, meta = (HideEditConditionToggle, EditCondition = "bPropertySetEnabled == true"))
	FQuat Rotation = FQuat::Identity;

	// Recenter the gizmo around the target position (without changing work plane), if it is "too far" (> 10 meters + max bounds dim) from that position currently
	void RecenterGizmoIfFar(FVector CenterPosition, double BoundsMaxDim, double TooFarDistance = 1000)
	{
		double DistanceTolSq = (BoundsMaxDim + TooFarDistance) * (BoundsMaxDim + TooFarDistance);
		if (FVector::DistSquared(CenterPosition, Position) > DistanceTolSq)
		{
			FVector Normal = Rotation.GetAxisZ();
			Position = CenterPosition - (CenterPosition - Position).ProjectOnToNormal(Normal);
		}
	}
};




UCLASS()
class HANDYMAN_API UHandyManSculptMaxBrushProperties : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	/** Specify maximum displacement height (relative to brush size) */
	UPROPERTY(EditAnywhere, Category = SculptMaxBrush, meta = (UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float MaxHeight = 0.5;

	/** Use maximum height from last brush stroke, regardless of brush size. Note that spatial brush falloff still applies.  */
	UPROPERTY(EditAnywhere, Category = SculptMaxBrush)
	bool bFreezeCurrentHeight = false;
};



#pragma endregion ToolSets

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManSculptTool : public UHandyManMeshSurfaceTool
{
	GENERATED_BODY()
	
protected:
	using FFrame3d = UE::Geometry::FFrame3d;
public:

	virtual void RegisterActions(FInteractiveToolActionSet& ActionSet) override;

	virtual void SetWorld(UWorld* World);

	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual void Render(IToolsContextRenderAPI* RenderAPI) override;

	// UMeshSurfacePointTool API
	virtual bool HitTest(const FRay& Ray, FHitResult& OutHit) override;
	virtual void OnBeginDrag(const FRay& Ray) override;
	virtual void OnUpdateDrag(const FRay& Ray) override;
	virtual void OnEndDrag(const FRay& Ray) override;
	virtual void OnCancelDrag() override;
	// end UMeshSurfacePointTool API

protected:
	virtual void OnTick(float DeltaTime) override;

	
	virtual void OnCompleteSetup();
	virtual void OnBeginStroke(const FRay& WorldRay) { check(false); }
	virtual void OnEndStroke() { check(false); }
	virtual void OnCancelStroke() { check(false); }

public:


	
	
	/** Properties that control brush size/etc */
	UPROPERTY()
	TObjectPtr<UHandyManSculptBrushProperties> BrushProperties;

	/** Properties for 3D workplane / gizmo */
	UPROPERTY()
	TObjectPtr<UHandyManWorkPlaneProperties> GizmoProperties;


protected:
	FViewCameraState CameraState;

	/** Initial transformation on target mesh */
	UE::Geometry::FTransformSRT3d InitialTargetTransform;
	/** Active transformation on target mesh, includes baked scale */
	UE::Geometry::FTransformSRT3d CurTargetTransform;

	FRay3d GetLocalRay(const FRay& WorldRay) const;


	/**
	 * Subclass must implement this and return relevant rendering component
	 */
	virtual UBaseDynamicMeshComponent* GetSculptMeshComponent() { check(false); return nullptr; }

	virtual FDynamicMesh3* GetSculptMesh() { return GetSculptMeshComponent()->GetMesh(); }
	virtual const FDynamicMesh3* GetSculptMesh() const { return const_cast<UHandyManSculptTool*>(this)->GetSculptMeshComponent()->GetMesh(); }

	virtual FDynamicMesh3* GetBaseMesh() { check(false); return nullptr; }
	virtual const FDynamicMesh3* GetBaseMesh() const { check(false); return nullptr; }

	// For any subclass where this returns false, BrushProperties will not be automatically saved/restored, so the class won't use BrushProperties changes made in other tools.
	virtual bool SharesBrushPropertiesChanges() const { return true; }


	/**
	 * Subclass calls this to set up editing component
	 */
	virtual void InitializeSculptMeshComponent();


	/**
	 * Subclass can override this to change what results are written.
	 * Default is to apply a default vertex positions update to the target object.
	 */
	virtual void CommitResult(UBaseDynamicMeshComponent* Component, bool bModifiedTopology);


	//
	// Brush Types
	//
public:
	struct FHandyManBrushTypeInfo
	{
		FText Name;
		int32 Identifier;
	};
	const TArray<FHandyManBrushTypeInfo>& GetRegisteredPrimaryBrushTypes() const { return RegisteredPrimaryBrushTypes; }
	const TArray<FHandyManBrushTypeInfo>& GetRegisteredSecondaryBrushTypes() const { return RegisteredSecondaryBrushTypes; }

protected:
	TArray<FHandyManBrushTypeInfo> RegisteredPrimaryBrushTypes;
	TArray<FHandyManBrushTypeInfo> RegisteredSecondaryBrushTypes;

	UPROPERTY()
	TMap<int32, TObjectPtr<UHandyManMeshSculptBrushOpProps>> BrushOpPropSets;

	TMap<int32, TUniquePtr<FHandyManMeshSculptBrushOpFactory>> BrushOpFactories;

	UPROPERTY()
	TMap<int32, TObjectPtr<UHandyManMeshSculptBrushOpProps>> SecondaryBrushOpPropSets;

	TMap<int32, TUniquePtr<FHandyManMeshSculptBrushOpFactory>> SecondaryBrushOpFactories;

	void RegisterBrushType(int32 Identifier, FText Name, TUniquePtr<FHandyManMeshSculptBrushOpFactory> Factory, UHandyManMeshSculptBrushOpProps* PropSet);
	void RegisterSecondaryBrushType(int32 Identifier, FText Name, TUniquePtr<FHandyManMeshSculptBrushOpFactory> Factory, UHandyManMeshSculptBrushOpProps* PropSet);

	virtual void SaveAllBrushTypeProperties(UInteractiveTool* SaveFromTool);
	virtual void RestoreAllBrushTypeProperties(UInteractiveTool* RestoreToTool);

protected:
	TUniquePtr<FHandyManMeshSculptBrushOp> PrimaryBrushOp;
	UHandyManMeshSculptBrushOpProps* PrimaryVisiblePropSet = nullptr;		// BrushOpPropSets prevents GC of this

	TUniquePtr<FHandyManMeshSculptBrushOp> SecondaryBrushOp;
	UHandyManMeshSculptBrushOpProps* SecondaryVisiblePropSet = nullptr;

	bool bBrushOpPropsVisible = true;

	void SetActivePrimaryBrushType(int32 Identifier);
	void SetActiveSecondaryBrushType(int32 Identifier);
	virtual TUniquePtr<FHandyManMeshSculptBrushOp>& GetActiveBrushOp();
	void SetBrushOpPropsVisibility(bool bVisible);

	//
	// Falloff types
	//
public:
	struct FFalloffTypeInfo
	{
		FText Name;
		FString StringIdentifier;
		int32 Identifier;
	};
	const TArray<FFalloffTypeInfo>& GetRegisteredPrimaryFalloffTypes() const { return RegisteredPrimaryFalloffTypes; }

	/** Set the active falloff type for the primary brush */
	virtual void SetPrimaryFalloffType(EHandyManMeshSculptFalloffType Falloff);

protected:
	TSharedPtr<FHandyManMeshSculptFalloffFunc> PrimaryFalloff;

	TArray<FFalloffTypeInfo> RegisteredPrimaryFalloffTypes;
	void RegisterStandardFalloffTypes();


	//
	// Brush Size
	//
protected:
	UE::Geometry::FInterval1d BrushRelativeSizeRange;
	double CurrentBrushRadius = 1.0;
	virtual void InitializeBrushSizeRange(const UE::Geometry::FAxisAlignedBox3d& TargetBounds);
	virtual void CalculateBrushRadius();
	virtual double GetCurrentBrushRadius() const { return CurrentBrushRadius; }

	double CurrentBrushFalloff = 0.5;
	virtual double GetCurrentBrushFalloff() const { return CurrentBrushFalloff; }

	double ActivePressure = 1.0;
	virtual double GetActivePressure() const { return ActivePressure; }

	virtual double GetCurrentBrushStrength();
	virtual double GetCurrentBrushDepth();

public:
	virtual void IncreaseBrushRadiusAction();
	virtual void DecreaseBrushRadiusAction();
	virtual void IncreaseBrushRadiusSmallStepAction();
	virtual void DecreaseBrushRadiusSmallStepAction();


	// client currently needs to implement these...
	virtual void IncreaseBrushSpeedAction() {}
	virtual void DecreaseBrushSpeedAction() {}
	virtual void NextBrushModeAction() {}
	virtual void PreviousBrushModeAction() {}


public:
	// IInteractiveToolCameraFocusAPI override to focus on brush w/ 'F'
	virtual FBox GetWorldSpaceFocusBox() override;



	//
	// Brush/Stroke stuff
	//
protected:
	FFrame3d LastBrushFrameWorld;
	FFrame3d LastBrushFrameLocal;
	int32 LastBrushTriangleID;

	const FFrame3d& GetBrushFrameWorld() const { return LastBrushFrameWorld; }
	const FFrame3d& GetBrushFrameLocal() const { return LastBrushFrameLocal; }
	int32 GetBrushTriangleID() const { return LastBrushTriangleID; }
	void UpdateBrushFrameWorld(const FVector3d& NewPosition, const FVector3d& NewNormal);
	void AlignBrushToView();

	bool GetBrushCanHitBackFaces() const { return BrushProperties->bHitBackFaces; }

	/** @return hit triangle at ray position - subclass must implement this */
	virtual int32 FindHitSculptMeshTriangle(const FRay3d& LocalRay) { check(false); return IndexConstants::InvalidID; }
	/** @return hit triangle at ray position - subclass should implement this for most brushes */
	virtual int32 FindHitTargetMeshTriangle(const FRay3d& LocalRay) { check(false); return IndexConstants::InvalidID;	}

	virtual bool UpdateBrushPositionOnActivePlane(const FRay& WorldRay);
	virtual bool UpdateBrushPositionOnTargetMesh(const FRay& WorldRay, bool bFallbackToViewPlane);
	virtual bool UpdateBrushPositionOnSculptMesh(const FRay& WorldRay, bool bFallbackToViewPlane);



	//
	// Brush Target Plane is plane that some brushes move on
	//
protected:
	FFrame3d ActiveBrushTargetPlaneWorld;
	virtual void UpdateBrushTargetPlaneFromHit(const FRay& WorldRay, const FHitResult& Hit);


	//
	// Stroke Modifiers
	//
protected:
	bool bInStroke = false;
	bool bSmoothing = false;
	bool bInvert = false;
	virtual void SaveActiveStrokeModifiers();
	virtual bool InStroke() const { return bInStroke; }
	virtual bool GetInSmoothingStroke() const { return bSmoothing; }
	virtual bool GetInInvertStroke() const { return bInvert; }

	// when in a stroke, this function determines when a new stamp should be emitted, based on spacing and flow rate settings
	virtual void UpdateStampPendingState();

	// for tracking stroke time and length, to apply spacing and flow rate settings
	double ActiveStrokeTime = 0.0;
	double ActiveStrokePathArcLen = 0.0;
	int LastFlowTimeStamp = 0;
	int LastSpacingTimestamp = 0;
	virtual void ResetStrokeTime();
	virtual void AccumulateStrokeTime(float DeltaTime);

	//
	// Stamps
	//
protected:
	bool bIsStampPending = false;
	FRay PendingStampRay;
	FHandyManSculptBrushStamp HoverStamp;
	FHandyManSculptBrushStamp CurrentStamp;
	FHandyManSculptBrushStamp LastStamp;
	virtual void UpdateHoverStamp(const FFrame3d& StampFrameWorld);
	virtual bool IsStampPending() const { return bIsStampPending; }
	virtual const FRay& GetPendingStampRayWorld() const { return PendingStampRay;  }
	// Temporal Flow Rate defines the frequency of stamp placement. 1 is max rate, 0 is no stamps. Defaults to BrushProperties->FlowRate, but subclasses can re-use that setting for other things
	virtual float GetStampTemporalFlowRate() const;


	//
	// Stamp ROI Plane is a plane used by some brush ops
	//
protected:
	FFrame3d StampRegionPlane;
	virtual FFrame3d ComputeStampRegionPlane(const FFrame3d& StampFrame, const TArray<int32>& StampTriangles, bool bIgnoreDepth, bool bViewAligned, bool bInvDistFalloff = true);
	virtual FFrame3d ComputeStampRegionPlane(const FFrame3d& StampFrame, const TSet<int32>& StampTriangles, bool bIgnoreDepth, bool bViewAligned, bool bInvDistFalloff = true);


	// Stroke plane is a plane used by some brush ops
	//
protected:
	FFrame3d StrokePlane;
	virtual const FFrame3d& GetCurrentStrokeReferencePlane() const { return StrokePlane; }

	virtual void UpdateStrokeReferencePlaneForROI(const FFrame3d& StampFrame, const TArray<int32>& TriangleROI, bool bViewAligned);
	virtual void UpdateStrokeReferencePlaneFromWorkPlane();


	//
	// Display / Material
	//
public:
	UPROPERTY()
	TObjectPtr<UHandyManMeshEditingViewProperties> ViewProperties;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ActiveOverrideMaterial;

protected:
	virtual void SetViewPropertiesEnabled(bool bNewValue);
	virtual void UpdateWireframeVisibility(bool bNewValue);
	virtual void UpdateMaterialMode(EHandyManMeshEditingMaterialModes NewMode);
	virtual void UpdateFlatShadingSetting(bool bNewValue);
	virtual void UpdateColorSetting(FLinearColor NewColor);
	virtual void UpdateTransparentColorSetting(FLinearColor NewColor);
	virtual void UpdateOpacitySetting(double Opacity);
	virtual void UpdateTwoSidedSetting(bool bOn);
	virtual void UpdateCustomMaterial(TWeakObjectPtr<UMaterialInterface> NewMaterial);
	virtual void UpdateImageSetting(UTexture2D* NewImage);



	//
	// brush indicator
	//
protected:
	// subclasses should call this to create indicator in their ::Setup()
	virtual void InitializeIndicator();

	// Called by InitializeIndicator to create a mesh for the brush ROI indicator. Default is sphere.
	virtual UPreviewMesh* MakeBrushIndicatorMesh(UObject* Parent, UWorld* World);

	virtual void ConfigureIndicator(bool bVolumetric);
	virtual bool GetIsVolumetricIndicator();

	virtual void SetIndicatorVisibility(bool bVisible);
	virtual bool GetIndicatorVisibility() const;

protected:
	UPROPERTY()
	TObjectPtr<UHandyManBrushStampIndicator> BrushIndicator;

	UPROPERTY()
	bool bIsVolumetricIndicator;


	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BrushIndicatorMaterial;

	UPROPERTY()
	TObjectPtr<UPreviewMesh> BrushIndicatorMesh;



	//
	// Work Plane
	//
public:
	// plane gizmo
	UPROPERTY()
	TObjectPtr<UCombinedTransformGizmo> PlaneTransformGizmo;

	UPROPERTY()
	TObjectPtr<UTransformProxy> PlaneTransformProxy;

protected:
	virtual void UpdateWorkPlane();
	virtual bool ShowWorkPlane() const { return false; };

protected:
	TValueWatcher<FVector> GizmoPositionWatcher;
	TValueWatcher<FQuat> GizmoRotationWatcher;

	virtual void UpdateGizmoFromProperties();
	virtual void PlaneTransformChanged(UTransformProxy* Proxy, FTransform Transform);

	enum class EPendingWorkPlaneUpdate
	{
		NoUpdatePending,
		MoveToHitPositionNormal,
		MoveToHitPosition,
		MoveToHitPositionViewAligned
	};
	EPendingWorkPlaneUpdate PendingWorkPlaneUpdate;
	virtual void SetFixedSculptPlaneFromWorldPos(const FVector& Position, const FVector& Normal, EPendingWorkPlaneUpdate UpdateType);
	virtual void UpdateFixedSculptPlanePosition(const FVector& Position);
	virtual void UpdateFixedSculptPlaneRotation(const FQuat& Rotation);
	virtual void UpdateFixedPlaneGizmoVisibility(bool bVisible);

protected:
	UE::TransactionUtil::FLongTransactionTracker LongTransactions;

};
