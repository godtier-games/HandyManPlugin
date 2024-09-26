// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "PropertySets/AxisFilterPropertyType.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Operators/HandyManMeshBrushOperators.h"
#include "UObject/Object.h"
#include "HandyManMoveBrushOps.generated.h"

UCLASS()
class HANDYMAN_API UHandyManMoveBrushOpProps : public UHandyManMeshSculptBrushOpProps
{
	GENERATED_BODY()
public:
	/** Strength of the Brush */
	UPROPERTY(EditAnywhere, Category = MoveBrush, meta = (DisplayName = "Strength", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Strength = 1.0;

	/** Amount of falloff to apply */
	UPROPERTY(EditAnywhere, Category = MoveBrush, meta = (DisplayName = "Falloff", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Falloff = 0.5;

	/** Depth of Brush into surface along view ray */
	UPROPERTY(EditAnywhere, Category = MoveBrush, meta = (UIMin = "-0.5", UIMax = "0.5", ClampMin = "-1.0", ClampMax = "1.0"))
	float Depth = 0;

	virtual float GetStrength() override { return Strength; }
	virtual void SetStrength(float NewStrength) override { Strength = FMathf::Clamp(NewStrength, 0.0f, 1.0f); }
	virtual float GetFalloff() override { return Falloff; }
	virtual float GetDepth() override { return Depth; }

	/** Axis filters restrict mesh movement to World X/Y/Z axes */
	UPROPERTY(EditAnywhere, Category = MoveBrush)
	FModelingToolsAxisFilter AxisFilters;
};


class FHandyManMoveBrushOp : public FHandyManMeshSculptBrushOp
{
public:

	virtual void ApplyStamp(const FDynamicMesh3* Mesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override
	{
		const FVector3d& StampPos = Stamp.LocalFrame.Origin;

		double UsePower = Stamp.Power;
		FVector3d MoveVec = Stamp.LocalFrame.Origin - Stamp.PrevLocalFrame.Origin;
		
		UHandyManMoveBrushOpProps* Props = GetPropertySetAs<UHandyManMoveBrushOpProps>();

		// if we have axis constraints we want to apply them in world space, and then remap
		// that world vector to local space
		if (Props->AxisFilters.AnyAxisFiltered())
		{
			FVector3d WorldMoveVec = Stamp.WorldFrame.Origin - Stamp.PrevWorldFrame.Origin;
			WorldMoveVec = MoveVec.Length() * UE::Geometry::Normalized(WorldMoveVec);		// apply local space scaling
			WorldMoveVec.X = (Props->AxisFilters.bAxisX) ? WorldMoveVec.X : 0;
			WorldMoveVec.Y = (Props->AxisFilters.bAxisY) ? WorldMoveVec.Y : 0;
			WorldMoveVec.Z = (Props->AxisFilters.bAxisZ) ? WorldMoveVec.Z : 0;
			MoveVec = Stamp.LocalFrame.FromFrameVector(Stamp.WorldFrame.ToFrameVector(WorldMoveVec));
		}

		ParallelFor(Vertices.Num(), [&](int32 k)
		{
			int32 VertIdx = Vertices[k];
			FVector3d OrigPos = Mesh->GetVertex(VertIdx);

			double Falloff = GetFalloff().Evaluate(Stamp, OrigPos);

			FVector3d NewPos = OrigPos + Falloff * MoveVec;
			NewPositionsOut[k] = NewPos;
		});
	}


	virtual EHandyManSculptBrushOpTargetType GetBrushTargetType() const override
	{
		return EHandyManSculptBrushOpTargetType::ActivePlane;
	}

	virtual bool IgnoreZeroMovements() const override
	{
		return true;
	}
};
