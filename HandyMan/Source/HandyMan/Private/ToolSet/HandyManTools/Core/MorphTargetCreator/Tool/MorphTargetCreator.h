// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveToolQueryInterfaces.h"
#include "Components/DynamicMeshComponent.h"
#include "DynamicMesh/DynamicMeshOctree3.h"
#include "Image/ImageBuilder.h"
#include "Parameterization/MeshPlanarSymmetry.h"
#include "Polygroups/PolygroupSet.h"
#include "ToolSet/HandyManBaseClasses/HandyManClickDragTool.h"
#include "ToolSet/HandyManTools/Core/MorphTargetCreator/DataTypes/MorphTargetCreatorTypes.h"
#include "ToolSet/HandyManTools/Core/SculptTool/DataTypes/HandyManSculptingTypes.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Tool/HandyManSculptTool.h"
#include "Util/UniqueIndexSet.h"
#include "MorphTargetCreator.generated.h"

#pragma region PROPERTIES

class AMorphTargetCreatorProxyActor;

UCLASS()
class HANDYMAN_API UMorphTargetProperties : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:

	TWeakObjectPtr<UScriptableInteractiveTool> GetParentTool() const {return ParentTool;};

	UPROPERTY(EditAnywhere, Category = Morphs)
	TObjectPtr<USkeletalMesh> TargetMesh;
	
	/** Primary Brush Mode */
	UPROPERTY(EditAnywhere, Category = Morphs)
	TArray<FName> MorphTargets;
	
	UPROPERTY(VisibleAnywhere, Category = Morphs)
	TMap<FName, UDynamicMesh*> Meshes;


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	

	
};

UCLASS()
class HANDYMAN_API UMorphTargetBrushSculptProperties : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:

	TWeakObjectPtr<UScriptableInteractiveTool> GetParentTool() const {return ParentTool;};
	
	/** Primary Brush Mode */
	UPROPERTY(EditAnywhere, Category = Sculpting, meta = (DisplayName = "Brush"))
	EHandyManBrushType PrimaryBrushType = EHandyManBrushType::Offset;

	/** Primary Brush Falloff Type, multiplied by Alpha Mask where applicable */
	UPROPERTY(EditAnywhere, Category = Sculpting, meta = (DisplayName = "Falloff"))
	EHandyManMeshSculptFalloffType PrimaryFalloffType = EHandyManMeshSculptFalloffType::Smooth;

	/** Filter applied to Stamp Region Triangles, based on first Stroke Stamp */
	UPROPERTY(EditAnywhere, Category = Sculpting, meta = (DisplayName = "Region"))
	EHandyManBrushFilterType BrushFilter = EHandyManBrushFilterType::None;

	/** When Freeze Target is toggled on, the Brush Target Surface will be Frozen in its current state, until toggled off. Brush strokes will be applied relative to the Target Surface, for applicable Brushes */
	UPROPERTY(EditAnywhere, Category = Sculpting, meta = (EditCondition = "PrimaryBrushType == EMorphTargetBrushType::Offset || PrimaryBrushType == EMorphTargetBrushType::SculptMax || PrimaryBrushType == EMorphTargetBrushType::SculptView || PrimaryBrushType == EMorphTargetBrushType::Pinch || PrimaryBrushType == EMorphTargetBrushType::Resample" ))
	bool bFreezeTarget = false;
	
};



/**
 * Tool Properties for a brush alpha mask
 */
UCLASS()
class HANDYMAN_API UMorphTargetBrushAlphaProperties : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:

	TWeakObjectPtr<UScriptableInteractiveTool> GetParentTool() const {return ParentTool;};
	
	/** Alpha mask applied to brush stamp. Red channel is used. */
	UPROPERTY(EditAnywhere, Category = Alpha, meta = (DisplayName = "Alpha Mask"))
	TObjectPtr<UTexture2D> Alpha = nullptr;

	/** Alpha is rotated by this angle, inside the brush stamp frame (vertically aligned) */
	UPROPERTY(EditAnywhere, Category = Alpha, meta = (DisplayName = "Angle", UIMin = "-180.0", UIMax = "180.0", ClampMin = "-360.0", ClampMax = "360.0"))
	float RotationAngle = 0.0;

	/** If true, a random angle in +/- RandomRange is added to Rotation angle for each stamp */
	UPROPERTY(EditAnywhere, Category = Alpha, AdvancedDisplay)
	bool bRandomize = false;

	/** Bounds of random generation (positive and negative) for randomized stamps */
	UPROPERTY(EditAnywhere, Category = Alpha, AdvancedDisplay, meta = (UIMin = "0.0", UIMax = "180.0"))
	float RandomRange = 180.0;
	
};




UCLASS()
class HANDYMAN_API UMorphTargetMeshSymmetryProperties : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	/** Enable/Disable symmetric sculpting. This option will not be available if symmetry cannot be detected, or a non-symmetric edit has been made */
	UPROPERTY(EditAnywhere, Category = Symmetry, meta = (HideEditConditionToggle, EditCondition = bSymmetryCanBeEnabled))
	bool bEnableSymmetry = true;

	// this flag is set/updated by the Tool to enable/disable the bEnableSymmetry toggle
	UPROPERTY(meta = (TransientToolProperty))
	bool bSymmetryCanBeEnabled = false;
};

#pragma endregion




/**
 * Tool Builder
 */
UCLASS()
class HANDYMAN_API UMorphTargetCreatorToolBuilder : public UHandyManMeshSurfaceToolBuilder
{
	GENERATED_BODY()
	
public:
	
	UMorphTargetCreatorToolBuilder(const FObjectInitializer& ObjectInitializer);
};

/**
 * Using a TMAP add a new morph target to the input mesh using sculpting tools
 */
UCLASS()
class HANDYMAN_API UMorphTargetCreator : public UHandyManSculptTool, public IInteractiveToolManageGeometrySelectionAPI
{
	GENERATED_BODY()

public:
	
	UMorphTargetCreator();

	
	void SpawnActorInstance(const FTransform& SpawnTransform);
	virtual void InitializeSculptMeshComponent() override;
	void TriggerToolStartUp(EToolsFrameworkOutcomePins PropertyCreationOutcome);
	virtual void UpdateMaterialMode(EHandyManMeshEditingMaterialModes NewMode) override;
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual void Render(IToolsContextRenderAPI* RenderAPI) override;

	virtual void OnTick(float DeltaTime) override;

	virtual bool HasCancel() const override { return true; }
	virtual bool HasAccept() const override { return true; }
	virtual bool CanAccept() const override;

	virtual bool HitTest(const FRay& Ray, FHitResult& OutHit) override;
	virtual FInputRayHit TestIfCanBeginClickDrag_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;

	virtual bool OnUpdateHover(const FInputDeviceRay& DevicePos) override;

	virtual void OnPropertyModified(UObject* PropertySet, FProperty* Property) override;

	// IInteractiveToolManageGeometrySelectionAPI -- this tool won't update external geometry selection or change selection-relevant mesh IDs
	virtual bool IsInputSelectionValidOnOutput() override
	{
		return true;
	}

	bool bHasToolStarted = false;

	UPROPERTY()
	TObjectPtr<UDynamicMeshComponent> DynamicMeshComponent;

protected:

	UPROPERTY()
	TObjectPtr<UMorphTargetProperties> MorphTargetProperties;

private:
	
	UPROPERTY()
	TObjectPtr<AMorphTargetCreatorProxyActor> TargetActor;

	

public:

	TMap<FName, UDynamicMesh*> GetMorphTargetMeshMap() const;
	void RemoveMorphTargetMesh(FName MorphTargetMeshName);
	void RemoveAllMorphTargetMeshes();
	void CreateMorphTargetMesh(FName MorphTargetMeshName);


#pragma region SCULPTING LOGIC
	public:

	/** Properties that control sculpting*/
	UPROPERTY()
	TObjectPtr<UMorphTargetBrushSculptProperties> SculptProperties;

	UPROPERTY()
	TObjectPtr<UMorphTargetBrushAlphaProperties> AlphaProperties;

	UPROPERTY()
	TObjectPtr<UTexture2D> BrushAlpha;

	UPROPERTY()
	TObjectPtr<UMorphTargetMeshSymmetryProperties> SymmetryProperties;

public:
	virtual void IncreaseBrushSpeedAction() override;
	virtual void DecreaseBrushSpeedAction() override;

	virtual void UpdateBrushAlpha(UTexture2D* NewAlpha);

	virtual void SetActiveBrushType(int32 Identifier);
	virtual void SetActiveFalloffType(int32 Identifier);
	virtual void SetRegionFilterType(int32 Identifier);

	/**
	* OnDetailsPanelRequestRebuild is broadcast by the tool when it detects it needs to have it's details panel rebuilt outside
	* of normal rebuilding triggers, such as changing property set objects. This is useful in rare circumstances, such as when
	* the tool is using detail customizations and tool properties are changed outside of user interactions, such as via tool
	* preset loading. In these cases, the detail customization widgets might not be updated properly without rebuilding the details
	* panel completely.
	*/

	DECLARE_MULTICAST_DELEGATE(OnInteractiveToolDetailsPanelRequestRebuild);
	OnInteractiveToolDetailsPanelRequestRebuild OnDetailsPanelRequestRebuild;

protected:
	// UMeshSculptToolBase API
	virtual void InitializeIndicator() override;
	virtual UPreviewMesh* MakeBrushIndicatorMesh(UObject* Parent, UWorld* World) override;

	virtual UBaseDynamicMeshComponent* GetSculptMeshComponent() override { return DynamicMeshComponent; }
	virtual FDynamicMesh3* GetBaseMesh() override{ return &BaseMesh; }
	virtual const FDynamicMesh3* GetBaseMesh() const override{ return &BaseMesh; }

	virtual int32 FindHitSculptMeshTriangle(const FRay3d& LocalRay) override;
	virtual int32 FindHitTargetMeshTriangle(const FRay3d& LocalRay) override;
	bool IsHitTriangleBackFacing(int32 TriangleID, const FDynamicMesh3* QueryMesh);

	virtual void UpdateHoverStamp(const FFrame3d& StampFrameWorld) override;

	virtual void OnBeginStroke(const FRay& WorldRay) override;
	virtual void OnEndStroke() override;
	virtual void OnCancelStroke() override;
	// end UMeshSculptToolBase API
	

	// realtime visualization
	void OnDynamicMeshComponentChanged(UDynamicMeshComponent* Component, const FMeshVertexChange* Change, bool bRevert);
	FDelegateHandle OnDynamicMeshComponentChangedHandle;

	void UpdateBrushType(EHandyManBrushType BrushType);

	TUniquePtr<UE::Geometry::FPolygroupSet> ActiveGroupSet;
	TArray<int32> TriangleComponentIDs;

	int32 InitialStrokeTriangleID = -1;

	TSet<int32> AccumulatedTriangleROI;
	bool bUndoUpdatePending = false;
	TFuture<bool> UndoNormalsFuture;
	TFuture<bool> UndoUpdateOctreeFuture;
	TFuture<bool> UndoUpdateBaseMeshFuture;
	TArray<int> NormalsBuffer;
	void WaitForPendingUndoRedo();

	TArray<uint32> OctreeUpdateTempBuffer;
	TArray<bool> OctreeUpdateTempFlagBuffer;
	TFuture<void> StampUpdateOctreeFuture;
	bool bStampUpdatePending = false;
	void WaitForPendingStampUpdate();

	TArray<int> RangeQueryTriBuffer;
	UE::Geometry::FUniqueIndexSet VertexROIBuilder;
	UE::Geometry::FUniqueIndexSet TriangleROIBuilder;
	TArray<UE::Geometry::FIndex3i> TriangleROIInBuf;
	TArray<int> VertexROI;
	TArray<int> TriangleROIArray;
	void UpdateROI(const FVector3d& BrushPos);

	UE::Geometry::FUniqueIndexSet NormalsROIBuilder;
	TArray<std::atomic<bool>> NormalsFlags;		// set of per-vertex or per-element-id flags that indicate
												// whether normal needs recompute. Fast to do it this way
												// than to use a TSet or UniqueIndexSet...

	bool bTargetDirty;

	EHandyManBrushType PendingStampType = EHandyManBrushType::Smooth;

	bool UpdateStampPosition(const FRay& WorldRay);
	TFuture<void> ApplyStamp();

	FRandomStream StampRandomStream;

	FDynamicMesh3 BaseMesh;
	UE::Geometry::FDynamicMeshOctree3 BaseMeshSpatial;
	TArray<int32> BaseMeshIndexBuffer;
	bool bCachedFreezeTarget = false;
	void UpdateBaseMesh(const TSet<int32>* TriangleROI = nullptr);
	bool GetBaseMeshNearest(int32 VertexID, const FVector3d& Position, double SearchRadius, FVector3d& TargetPosOut, FVector3d& TargetNormalOut);
	TFunction<bool(int32, const FVector3d&, double MaxDist, FVector3d&, FVector3d&)> BaseMeshQueryFunc;

	UE::Geometry::FDynamicMeshOctree3 Octree;

	bool UpdateBrushPosition(const FRay& WorldRay);

	double SculptMaxFixedHeight = -1.0;

	bool bHaveBrushAlpha = false;
	UE::Geometry::TImageBuilder<FVector4f> BrushAlphaValues;
	UE::Geometry::FImageDimensions BrushAlphaDimensions;
	double SampleBrushAlpha(const FHandyManSculptBrushStamp& Stamp, const FVector3d& Position) const;

	TArray<FVector3d> ROIPositionBuffer;
	TArray<FVector3d> ROIPrevPositionBuffer;

	TPimplPtr<UE::Geometry::FMeshPlanarSymmetry> Symmetry;
	bool bMeshSymmetryIsValid = false;
	void TryToInitializeSymmetry();
	friend class FMorphTargetNonSymmetricChange;
	virtual void UndoRedo_RestoreSymmetryPossibleState(bool bSetToValue);

	bool bApplySymmetry = false;
	TArray<int> SymmetricVertexROI;
	TArray<FVector3d> SymmetricROIPositionBuffer;
	TArray<FVector3d> SymmetricROIPrevPositionBuffer;

	FMeshVertexChangeBuilder* ActiveVertexChange = nullptr;
	void BeginChange();
	void EndChange();


protected:
	virtual bool ShowWorkPlane() const override { return SculptProperties->PrimaryBrushType == EHandyManBrushType::FixedPlane; }


	
#pragma endregion 
};
