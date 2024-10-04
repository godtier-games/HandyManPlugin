#pragma once

#include "DynamicMesh/DynamicMesh3.h"
#include "Async/ParallelFor.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Operators/HandyManMeshBrushOperators.h"
#include "HandyManPlaneBrushOps.generated.h"





UCLASS()
class HANDYMAN_API UHandyManBasePlaneBrushOpProps : public UHandyManMeshSculptBrushOpProps
{
	GENERATED_BODY()
public:
	virtual EHandyManPlaneBrushSideMode GetWhichSide() { return EHandyManPlaneBrushSideMode::BothSides; }
};

UCLASS()
class HANDYMAN_API UHandyManPlaneBrushOpProps : public UHandyManBasePlaneBrushOpProps
{
	GENERATED_BODY()
public:
	/** Strength of the Brush */
	UPROPERTY(EditAnywhere, Category = PlaneBrush, meta = (DisplayName = "Strength", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Strength = 0.5;

	/** Amount of falloff to apply */
	UPROPERTY(EditAnywhere, Category = PlaneBrush, meta = (DisplayName = "Falloff", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Falloff = 0.5;

	/** Depth of Brush into surface along surface normal */
	UPROPERTY(EditAnywhere, Category = PlaneBrush, meta = (UIMin = "-0.5", UIMax = "0.5", ClampMin = "-1.0", ClampMax = "1.0"))
	float Depth = 0;

	/** Control whether effect of brush should be limited to one side of the Plane  */
	UPROPERTY(EditAnywhere, Category = PlaneBrush)
	EHandyManPlaneBrushSideMode WhichSide = EHandyManPlaneBrushSideMode::BothSides;

	virtual float GetStrength() override { return Strength; }
	virtual void SetStrength(float NewStrength) override { Strength = FMathf::Clamp(NewStrength, 0.0f, 1.0f); }
	virtual float GetFalloff() override { return Falloff; }
	virtual float GetDepth() override { return Depth; }
	virtual EHandyManPlaneBrushSideMode GetWhichSide() { return WhichSide; }
};


UCLASS()
class HANDYMAN_API UHandyManViewAlignedPlaneBrushOpProps : public UHandyManBasePlaneBrushOpProps
{
	GENERATED_BODY()
public:
	/** Strength of the Brush */
	UPROPERTY(EditAnywhere, Category = ViewPlaneBrush, meta = (DisplayName = "Strength", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Strength = 0.5;

	/** Amount of falloff to apply */
	UPROPERTY(EditAnywhere, Category = ViewPlaneBrush, meta = (DisplayName = "Falloff", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Falloff = 0.5;

	/** Depth of Brush into surface along view ray */
	UPROPERTY(EditAnywhere, Category = ViewPlaneBrush, meta = (UIMin = "-0.5", UIMax = "0.5", ClampMin = "-1.0", ClampMax = "1.0"))
	float Depth = 0;

	/** Control whether effect of brush should be limited to one side of the Plane  */
	UPROPERTY(EditAnywhere, Category = ViewPlaneBrush)
	EHandyManPlaneBrushSideMode WhichSide = EHandyManPlaneBrushSideMode::BothSides;

	virtual float GetStrength() override { return Strength; }
	virtual void SetStrength(float NewStrength) override { Strength = FMathf::Clamp(NewStrength, 0.0f, 1.0f); }
	virtual float GetFalloff() override { return Falloff; }
	virtual float GetDepth() override { return Depth; }
	virtual EHandyManPlaneBrushSideMode GetWhichSide() { return WhichSide; }
};


UCLASS()
class HANDYMAN_API UHandyManFixedPlaneBrushOpProps : public UHandyManBasePlaneBrushOpProps
{
	GENERATED_BODY()
public:
	/** Strength of the Brush */
	UPROPERTY(EditAnywhere, Category = FixedPlaneBrush, meta = (DisplayName = "Strength", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Strength = 0.5;

	/** Amount of falloff to apply */
	UPROPERTY(EditAnywhere, Category = FixedPlaneBrush, meta = (DisplayName = "Falloff", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Falloff = 0.5;

	/** Depth of Brush into surface relative to plane */
	UPROPERTY(EditAnywhere, Category = FixedPlaneBrush, meta = (UIMin = "-0.5", UIMax = "0.5", ClampMin = "-1.0", ClampMax = "1.0"))
	float Depth = 0;

	/** Control whether effect of brush should be limited to one side of the Plane  */
	UPROPERTY(EditAnywhere, Category = FixedPlaneBrush)
	EHandyManPlaneBrushSideMode WhichSide = EHandyManPlaneBrushSideMode::BothSides;

	virtual float GetStrength() override { return Strength; }
	virtual void SetStrength(float NewStrength) override { Strength = FMathf::Clamp(NewStrength, 0.0f, 1.0f); }
	virtual float GetFalloff() override { return Falloff; }
	virtual float GetDepth() override { return Depth; }
	virtual EHandyManPlaneBrushSideMode GetWhichSide() { return WhichSide; }
};



class FHandyManPlaneBrushOp : public FHandyManMeshSculptBrushOp
{

public:
	double BrushSpeedTuning = 6.0;

	FFrame3d StrokePlane;

	virtual void BeginStroke(const FDynamicMesh3* Mesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& InitialVertices) override
	{
		StrokePlane = CurrentOptions.ConstantReferencePlane;
	}

	virtual void ApplyStamp(const FDynamicMesh3* Mesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override
	{
		static const double PlaneSigns[3] = { 0, -1, 1 };
		UHandyManBasePlaneBrushOpProps* Props = GetPropertySetAs<UHandyManBasePlaneBrushOpProps>();
		double PlaneSign = PlaneSigns[(int32)Props->GetWhichSide()];

		const FVector3d& StampPos = Stamp.LocalFrame.Origin;

		double UseSpeed = Stamp.Power * Stamp.Radius * Stamp.DeltaTime * BrushSpeedTuning;

		ParallelFor(Vertices.Num(), [&](int32 k)
		{
			int32 VertIdx = Vertices[k];
			FVector3d OrigPos = Mesh->GetVertex(VertIdx);
			FVector3d PlanePos = StrokePlane.ToPlane(OrigPos, 2);
			FVector3d Delta = PlanePos - OrigPos;

			double Dot = Delta.Dot(StrokePlane.Z());
			FVector3d NewPos = OrigPos;
			if (Dot * PlaneSign >= 0)
			{
				double Falloff = GetFalloff().Evaluate(Stamp, OrigPos);
				FVector3d MoveVec = Falloff * UseSpeed * Delta;
				double MaxDist = UE::Geometry::Normalize(Delta);
				NewPos = (MoveVec.SquaredLength() > MaxDist * MaxDist) ?
					PlanePos : (OrigPos + Falloff * MoveVec);
			}

			NewPositionsOut[k] = NewPos;
		});
	}

};

