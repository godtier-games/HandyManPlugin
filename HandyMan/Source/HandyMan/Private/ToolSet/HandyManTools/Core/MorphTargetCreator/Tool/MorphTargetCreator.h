// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManBaseClasses/HandyManClickDragTool.h"
#include "ToolSet/HandyManTools/Core/MorphTargetCreator/DataTypes/MorphTargetCreatorTypes.h"
#include "MorphTargetCreator.generated.h"

UCLASS()
class MESHMODELINGTOOLSEXP_API UVertexBrushSculptProperties : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	/** Primary Brush Mode */
	UPROPERTY(EditAnywhere, Category = Sculpting, meta = (DisplayName = "Brush"))
	EMorphTargetBrushType PrimaryBrushType = EMorphTargetBrushType::Offset;

	/** Primary Brush Falloff Type, multiplied by Alpha Mask where applicable */
	UPROPERTY(EditAnywhere, Category = Sculpting, meta = (DisplayName = "Falloff"))
	EMeshSculptFalloffType PrimaryFalloffType = EMeshSculptFalloffType::Smooth;

	/** Filter applied to Stamp Region Triangles, based on first Stroke Stamp */
	UPROPERTY(EditAnywhere, Category = Sculpting, meta = (DisplayName = "Region"))
	EMeshVertexSculptBrushFilterType BrushFilter = EMeshVertexSculptBrushFilterType::None;

	/** When Freeze Target is toggled on, the Brush Target Surface will be Frozen in its current state, until toggled off. Brush strokes will be applied relative to the Target Surface, for applicable Brushes */
	UPROPERTY(EditAnywhere, Category = Sculpting, meta = (EditCondition = "PrimaryBrushType == EMorphTargetBrushType::Offset || PrimaryBrushType == EMorphTargetBrushType::SculptMax || PrimaryBrushType == EMorphTargetBrushType::SculptView || PrimaryBrushType == EMorphTargetBrushType::Pinch || PrimaryBrushType == EMorphTargetBrushType::Resample" ))
	bool bFreezeTarget = false;

	// parent ref required for details customization
	UPROPERTY(meta = (TransientToolProperty))
	TWeakObjectPtr<UMeshVertexSculptTool> Tool;
};



/**
 * Tool Properties for a brush alpha mask
 */
UCLASS()
class MESHMODELINGTOOLSEXP_API UVertexBrushAlphaProperties : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
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
class MESHMODELINGTOOLSEXP_API UMeshSymmetryProperties : public UScriptableInteractiveToolPropertySet
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


/**
 * Using a TMAP add a new morph target to the input mesh using sculpting tools
 */
UCLASS()
class HANDYMAN_API UMorphTargetCreator : public UHandyManClickDragTool
{
	GENERATED_BODY()

public:

	UMorphTargetCreator();
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;


	/**
	 * @return true if all ToolTargets of this tool are still valid
	 */
	virtual bool AreAllTargetsValid() const
	{
		return Targets.Num() == 1  ? Targets[0]->IsValid() : false;
	}


public:
	virtual bool CanAccept() const override
	{
		return AreAllTargetsValid();
	}
};
