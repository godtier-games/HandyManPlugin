// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Operators/HandyManMeshBrushOperators.h"
#include "UObject/Object.h"
#include "HandyManMeshSculptBrushOps.generated.h"

UCLASS()
class HANDYMAN_API UHandyManStandardSculptBrushOpProps : public UHandyManMeshSculptBrushOpProps
{
	GENERATED_BODY()
public:
	/** Strength of the Brush */
	UPROPERTY(EditAnywhere, Category = SculptBrush, meta = (DisplayName = "Strength", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Strength = 0.5;

	/** Amount of falloff to apply */
	UPROPERTY(EditAnywhere, Category = SculptBrush, meta = (DisplayName = "Falloff", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Falloff = 1.0;

	virtual float GetStrength() override { return Strength; }
	virtual void SetStrength(float NewStrength) override { Strength = FMathf::Clamp(NewStrength, 0.0f, 1.0f); }
	virtual float GetFalloff() override { return Falloff; }
};

class FHandyManSurfaceSculptBrushOp : public FHandyManMeshSculptBrushOp
{
public:
	double BrushSpeedTuning = 6.0;

	typedef TUniqueFunction<bool(int32, const FVector3d&, double, FVector3d&, FVector3d&)> NearestQueryFuncType;

	NearestQueryFuncType BaseMeshNearestQueryFunc;

	FHandyManSurfaceSculptBrushOp(NearestQueryFuncType QueryFunc)
	{
		BaseMeshNearestQueryFunc = MoveTemp(QueryFunc);
	}

	virtual EHandyManSculptBrushOpTargetType GetBrushTargetType() const override
	{
		return EHandyManSculptBrushOpTargetType::TargetMesh;
	}
	virtual bool SupportsVariableSpacing() const override
	{
		return true;
	}

	virtual void ApplyStamp(const FDynamicMesh3* Mesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override
	{
		double UsePower = Stamp.Direction * Stamp.Power * Stamp.Radius * Stamp.DeltaTime * BrushSpeedTuning;
		double MaxOffset = Stamp.Radius;

		bool bHaveAlpha = Stamp.HasAlpha();

		ParallelFor(Vertices.Num(), [&](int32 k)
		{
			int32 VertIdx = Vertices[k];
			FVector3d OrigPos = Mesh->GetVertex(VertIdx);

			FVector3d BasePos, BaseNormal;
			bool bFoundBasePos = BaseMeshNearestQueryFunc(VertIdx, OrigPos, 4.0 * Stamp.Radius, BasePos, BaseNormal);
			if (bFoundBasePos == false)
			{
				NewPositionsOut[k] = OrigPos;
			}
			else
			{
				double Alpha = (bHaveAlpha) ? Stamp.StampAlphaFunc(Stamp, OrigPos) : 1.0;

				FVector3d MoveVec = UsePower * BaseNormal;
				double Falloff = GetFalloff().Evaluate(Stamp, OrigPos) * Alpha;
				FVector3d NewPos = OrigPos + Falloff * MoveVec;
				NewPositionsOut[k] = NewPos;
			}
		});
	}

};





UCLASS()
class HANDYMAN_API UHandyManViewAlignedSculptBrushOpProps : public UHandyManMeshSculptBrushOpProps
{
	GENERATED_BODY()
public:
	/** Strength of the Brush */
	UPROPERTY(EditAnywhere, Category = SculptToViewBrush, meta = (DisplayName = "Strength", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Strength = 0.5;

	/** Amount of falloff to apply */
	UPROPERTY(EditAnywhere, Category = SculptToViewBrush, meta = (DisplayName = "Falloff", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Falloff = 1.0;

	virtual float GetStrength() override { return Strength; }
	virtual void SetStrength(float NewStrength) override { Strength = FMathf::Clamp(NewStrength, 0.0f, 1.0f); }
	virtual float GetFalloff() override { return Falloff; }
};


class FHandyManViewAlignedSculptBrushOp : public FHandyManMeshSculptBrushOp
{
public:
	double BrushSpeedTuning = 6.0;

	typedef TUniqueFunction<bool(int32, const FVector3d&, double, FVector3d&, FVector3d&)> NearestQueryFuncType;

	NearestQueryFuncType BaseMeshNearestQueryFunc;

	FHandyManViewAlignedSculptBrushOp(NearestQueryFuncType QueryFunc)
	{
		BaseMeshNearestQueryFunc = MoveTemp(QueryFunc);
	}

	virtual EHandyManSculptBrushOpTargetType GetBrushTargetType() const override
	{
		return EHandyManSculptBrushOpTargetType::TargetMesh;
	}

	virtual bool GetAlignStampToView() const override
	{
		return true;
	}
	virtual bool SupportsVariableSpacing() const override
	{
		return true;
	}

	virtual void ApplyStamp(const FDynamicMesh3* Mesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override
	{
		FVector3d StampNormal = Stamp.LocalFrame.Z();

		double UsePower = Stamp.Direction * Stamp.Power * Stamp.Radius * Stamp.DeltaTime * BrushSpeedTuning;
		double MaxOffset = Stamp.Radius;

		bool bHaveAlpha = Stamp.HasAlpha();

		ParallelFor(Vertices.Num(), [&](int32 k)
		{
			int32 VertIdx = Vertices[k];
			FVector3d OrigPos = Mesh->GetVertex(VertIdx);

			FVector3d BasePos, BaseNormal;
			bool bFoundBasePos = BaseMeshNearestQueryFunc(VertIdx, OrigPos, 4.0 * Stamp.Radius, BasePos, BaseNormal);
			if (bFoundBasePos == false)
			{
				NewPositionsOut[k] = OrigPos;
			}
			else
			{
				double Alpha = (bHaveAlpha) ? Stamp.StampAlphaFunc(Stamp, OrigPos) : 1.0;

				FVector3d MoveVec = UsePower * StampNormal;
				double Falloff = GetFalloff().Evaluate(Stamp, OrigPos) * Alpha;
				FVector3d NewPos = OrigPos + Falloff * MoveVec;
				NewPositionsOut[k] = NewPos;
			}
		});
	}

};





UCLASS()
class HANDYMAN_API UHandyManSculptMaxBrushOpProps : public UHandyManMeshSculptBrushOpProps
{
	GENERATED_BODY()
public:
	/** Strength of the Brush */
	UPROPERTY(EditAnywhere, Category = SculptMaxBrush, meta = (DisplayName = "Strength", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Strength = 0.5;

	/** Amount of falloff to apply */
	UPROPERTY(EditAnywhere, Category = SculptMaxBrush, meta = (DisplayName = "Falloff", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Falloff = 0.5;

	/** Maximum height as fraction of brush size */
	UPROPERTY(EditAnywhere, Category = SculptMaxBrush, meta = (UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float MaxHeight = 0.5;

	/** If true, maximum height is defined using the FixedHeight constant instead of brush-relative size */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = SculptMaxBrush)
	bool bUseFixedHeight = false;

	/** Maximum height in world-space dimension */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = SculptMaxBrush)
	float FixedHeight = 0.0;


	virtual float GetStrength() override { return Strength; }
	virtual void SetStrength(float NewStrength) override { Strength = FMathf::Clamp(NewStrength, 0.0f, 1.0f); }
	virtual float GetFalloff() override { return Falloff; }
};


class FHandyManSurfaceMaxSculptBrushOp : public FHandyManMeshSculptBrushOp
{
public:
	double BrushSpeedTuning = 6.0;

	typedef TUniqueFunction<bool(int32, const FVector3d&, double, FVector3d&, FVector3d&)> NearestQueryFuncType;

	NearestQueryFuncType BaseMeshNearestQueryFunc;

	FHandyManSurfaceMaxSculptBrushOp(NearestQueryFuncType QueryFunc)
	{
		BaseMeshNearestQueryFunc = MoveTemp(QueryFunc);
	}

	virtual EHandyManSculptBrushOpTargetType GetBrushTargetType() const override
	{
		return EHandyManSculptBrushOpTargetType::TargetMesh;
	}
	virtual bool SupportsVariableSpacing() const override
	{
		return true;
	}

	virtual void ApplyStamp(const FDynamicMesh3* Mesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override
	{
		double UsePower = Stamp.Direction * Stamp.Power * Stamp.Radius * Stamp.DeltaTime * BrushSpeedTuning;

		UHandyManSculptMaxBrushOpProps* Props = GetPropertySetAs<UHandyManSculptMaxBrushOpProps>();
		double MaxOffset = (Props->bUseFixedHeight) ? Props->FixedHeight : (Props->MaxHeight * Stamp.Radius);

		bool bHaveAlpha = Stamp.HasAlpha();

		ParallelFor(Vertices.Num(), [&](int32 k)
		{
			int32 VertIdx = Vertices[k];
			FVector3d OrigPos = Mesh->GetVertex(VertIdx);

			FVector3d BasePos, BaseNormal;
			bool bFoundBasePos = BaseMeshNearestQueryFunc(VertIdx, OrigPos, 4.0 * Stamp.Radius, BasePos, BaseNormal);
			if (bFoundBasePos == false)
			{
				NewPositionsOut[k] = OrigPos;
			}
			else
			{
				double Alpha = (bHaveAlpha) ? Stamp.StampAlphaFunc(Stamp, OrigPos) : 1.0;

				FVector3d MoveVec = UsePower * BaseNormal;
				double Falloff = GetFalloff().Evaluate(Stamp, OrigPos) * Alpha;
				FVector3d NewPos = OrigPos + Falloff * MoveVec;

				FVector3d DeltaPos = NewPos - BasePos;
				if (DeltaPos.SquaredLength() > MaxOffset * MaxOffset)
				{
					UE::Geometry::Normalize(DeltaPos);
					NewPos = BasePos + MaxOffset * DeltaPos;
				}

				NewPositionsOut[k] = NewPos;
			}
		});
	}

};

