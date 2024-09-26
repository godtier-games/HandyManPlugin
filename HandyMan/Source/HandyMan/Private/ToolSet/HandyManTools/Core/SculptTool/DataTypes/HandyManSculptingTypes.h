// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HandyManSculptingTypes.generated.h"


/** Mesh Sculpting Brush Types */
UENUM()
enum class EHandyManMeshSculptFalloffType : uint8
{
	Smooth = 0,
	Linear = 1,
	Inverse = 2,
	Round = 3,
	BoxSmooth = 4,
	BoxLinear = 5,
	BoxInverse = 6,
	BoxRound = 7,

	LastValue UMETA(Hidden)
};


/**
 * Type of Brush Size currently active in FBrushToolRadius
 */
UENUM()
enum class EHandyManBrushToolSizeType : uint8
{
	/** Brush size is a dimensionless scale relative to the target object size */
	Adaptive = 0,
	/** Brush size is defined in world dimensions */
	World = 1
};

/**
 * FHandyManBrushToolRadius is used to define the size of 3D "brushes" used in (eg) sculpting tools.
 * The brush size can be defined in various ways.
 */
USTRUCT()
struct FHandyManBrushToolRadius
{
	GENERATED_BODY()

	/** Specify the type of brush size currently in use */
	UPROPERTY(EditAnywhere, Category = Brush)
	EHandyManBrushToolSizeType SizeType = EHandyManBrushToolSizeType::Adaptive;

	/** Adaptive brush size is used to interpolate between an object-specific minimum and maximum brush size */
	UPROPERTY(EditAnywhere, Category = Brush, AdvancedDisplay, meta = (EditCondition = "SizeType == EHandyManBrushToolSizeType::Adaptive",
		DisplayName = "Size", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "10.0"))
	float AdaptiveSize = 0.25;

	/** World brush size is a dimension in world coordinates */
	UPROPERTY(EditAnywhere, Category = Brush, AdvancedDisplay, meta = (EditCondition = "SizeType == EHandyManBrushToolSizeType::World",
		DisplayName = "World Radius", UIMin = "1.0", UIMax = "1000.0", ClampMin = "0.1", ClampMax = "50000.0"))
	float WorldRadius = 100.0;

	/**
	 * WorldSizeRange defines the min/max dimensions for Adaptive brush size
	 */
	TInterval<float> WorldSizeRange = TInterval<float>(1.0f, 1000.0f);

	//
	// util functions
	//

	/** Set the WorldSizeRange value and optionally clamp the WorldRadius based on this new range */
	void InitializeWorldSizeRange(TInterval<float> Range, bool bValidateWorldRadius = true);
	/** Return the set/calculated world-space radius for the current settings */
	float GetWorldRadius() const;
	/** Increase the current radius dimension by a fixed step (or a smaller fixed step) */
	void IncreaseRadius(bool bSmallStep);
	/** Decrease the current radius dimension by a fixed step (or a smaller fixed step) */
	void DecreaseRadius(bool bSmallStep);
};



/** Mesh Sculpting Brush Types */
UENUM()
enum class EHandyManBrushType : uint8
{
	/** Move vertices parallel to the view plane  */
	Move UMETA(DisplayName = "Move"),

	/** Grab Brush, fall-off alters the influence of the grab */
	PullKelvin UMETA(DisplayName = "Kelvin Grab"),

	/** Grab Brush that may generate cusps, fall-off alters the influence of the grab */
	PullSharpKelvin UMETA(DisplayName = "Sharp Kelvin Grab"),

	/** Smooth mesh vertices  */
	Smooth UMETA(DisplayName = "Smooth"),

	/** Smooth mesh vertices but only in direction of normal (Ctrl to invert) */
	SmoothFill UMETA(DisplayName = "SmoothFill"),

	/** Displace vertices along the average surface normal (Ctrl to invert) */
	Offset UMETA(DisplayName = "Sculpt (Normal)"),

	/** Displace vertices towards the camera viewpoint (Ctrl to invert) */
	SculptView UMETA(DisplayName = "Sculpt (Viewpoint)"),

	/** Displaces vertices along the average surface normal to a maximum height based on the brush size (Ctrl to invert) */
	SculptMax UMETA(DisplayName = "Sculpt Max"),

	/** Displace vertices along their vertex normals */
	Inflate UMETA(DisplayName = "Inflate"),

	/** Scale Brush will inflate or pinch radially from the center of the brush */
	ScaleKelvin UMETA(DisplayName = "Kelvin Scale"),

	/** Move vertices towards the center of the brush (Ctrl to push away)*/
	Pinch UMETA(DisplayName = "Pinch"),

	/** Twist Brush moves vertices in the plane perpendicular to the local mesh normal */
	TwistKelvin UMETA(DisplayName = "Kelvin Twist"),

	/** Move vertices towards the average plane of the brush stamp region */
	Flatten UMETA(DisplayName = "Flatten"),

	/** Move vertices towards a plane defined by the initial brush position  */
	Plane UMETA(DisplayName = "Plane (Normal)"),

	/** Move vertices towards a view-facing plane defined at the initial brush position */
	PlaneViewAligned UMETA(DisplayName = "Plane (Viewpoint)"),

	/** Move vertices towards a fixed plane in world space, positioned with a 3D gizmo */
	FixedPlane UMETA(DisplayName = "FixedPlane"),

	LastValue UMETA(Hidden)

};



/** Brush Triangle Filter Type */
UENUM()
enum class EHandyManBrushFilterType : uint8
{
	/** Do not filter brush area */
	None = 0,
	/** Only apply brush to triangles in the same connected mesh component/island */
	Component = 1,
	/** Only apply brush to triangles with the same PolyGroup */
	PolyGroup = 2
};