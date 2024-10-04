#pragma once
#include "CoreMinimal.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "ModelingUtilitiesDataTypes.generated.h"

class UDynamicMesh;
class USplineComponent;

UENUM(BlueprintType)
enum class ESweepShapeType : uint8
{
	Circle,
	Triangle,
	Box,
	Custom
};

USTRUCT(BlueprintType)
struct FSweepOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Options")
	TObjectPtr<UDynamicMesh> TargetMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Options")
	TObjectPtr<USplineComponent> Spline = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Options")
	bool bResampleCurve = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Options")
	int32 SampleSize = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Options")
	bool bProjectPointsToSurface = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Options")
	FVector2D StartEndRadius = FVector2D(1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Options")
	ESweepShapeType ShapeType = ESweepShapeType::Circle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="ShapeType == ESweepShapeType::Custom", EditConditionHides), Category = "Sweep Options")
	TObjectPtr<USplineComponent> CustomProfile = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="ShapeType != ESweepShapeType::Custom", EditConditionHides), Category = "Sweep Options")
	int32 ShapeSegments = 16;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Options")
	float ShapeRadius = 4.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="ShapeType == ESweepShapeType::Rectangle", EditConditionHides), Category = "Sweep Options")
	FVector2D ShapeDimensions = FVector2D(100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Options")
	float RotationAngleDeg = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options, meta = (DisplayName = "PolyGroup Mode"))
	EGeometryScriptPrimitivePolygroupMode PolygroupMode = EGeometryScriptPrimitivePolygroupMode::PerFace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bFlipOrientation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bEndCaps = false;

	/* If true, the target mesh will be reset to an empty mesh before the sweep operation
	 * Set this to false if you are sweeping multiple times on the same mesh
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bResetTargetMesh = true;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	EGeometryScriptPrimitiveUVMode UVMode = EGeometryScriptPrimitiveUVMode::Uniform;
	
};

USTRUCT(BlueprintType)
struct FSimpleCollisionOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider Options")
	TObjectPtr<UDynamicMesh> TargetMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider Options")
	TObjectPtr<USplineComponent> Spline = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider Options")
	bool bOffsetFromCenter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider Options")
	double Height = 50.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider Options")
	double Width = 50.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider Options")
	double ZOffset = 0.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider Options")
	double ErrorTolerance = 1.0;
	
};

UENUM(BlueprintType)
enum class EMeshBooleanShape : uint8
{
	/* Base shape is a cube or rectangle */
	Box,

	/* Base shape is cylindrical or spherical */
	Round,

	/* The input mesh is a box with an unequal amount of sides ( Like a pentagon or a rhombus )
	 * This will try to create a shape following that shape instead of just a box that fits the mesh
	 */
	NonUniformBox,

	/* Use the source mesh as the boolean shape. Useful for cutters that have both round and box shapes combined.
	 * This takes longer to compute and is not always accurate for all meshes.
	 * WARNING!! This will not create a closed shape from your mesh, it will just use the mesh as a boolean shape.
	 */
	Exact
};
