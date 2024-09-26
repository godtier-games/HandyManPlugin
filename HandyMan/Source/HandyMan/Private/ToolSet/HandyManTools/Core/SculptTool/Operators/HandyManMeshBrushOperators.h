// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FrameTypes.h"
#include "ScriptableInteractiveTool.h"
#include "UObject/Object.h"
#include "HandyManMeshBrushOperators.generated.h"

PREDECLARE_USE_GEOMETRY_CLASS(FDynamicMesh3);
using UE::Geometry::FFrame3d;
using UE::Geometry::FMatrix3d;

enum class EHandyManSculptBrushOpTargetType : uint8
{
	SculptMesh,
	TargetMesh,
	ActivePlane
};


UENUM()
enum class EHandyManPlaneBrushSideMode : uint8
{
	BothSides = 0,
	PushDown = 1,
	PullTowards = 2
};


struct HANDYMAN_API FHandyManSculptBrushStamp
{
	FFrame3d WorldFrame;
	FFrame3d LocalFrame;
	double Radius;
	double Falloff;
	double Power;
	double Direction;
	double Depth;
	double DeltaTime;

	FFrame3d PrevWorldFrame;
	FFrame3d PrevLocalFrame;

	FDateTime TimeStamp;

	// only initialized if current op requires it
	FFrame3d RegionPlane;

	// stamp alpha
	TFunction<double(const FHandyManSculptBrushStamp& Stamp, const FVector3d& Position)> StampAlphaFunc;
	bool HasAlpha() const { return !!StampAlphaFunc; }

	FHandyManSculptBrushStamp()
	{
		TimeStamp = FDateTime::Now();
	}
};


struct HANDYMAN_API FHandyManSculptBrushOptions
{
	//bool bPreserveUVFlow = false;

	FFrame3d ConstantReferencePlane;
};


class HANDYMAN_API FHandyManMeshSculptFalloffFunc
{
public:
	TUniqueFunction<double(const FHandyManSculptBrushStamp& StampInfo, const FVector3d& Position)> FalloffFunc;

	inline double Evaluate(const FHandyManSculptBrushStamp& StampInfo, const FVector3d& Position) const
	{
		return FalloffFunc(StampInfo, Position);
	}
};



UCLASS()
class HANDYMAN_API UHandyManMeshSculptBrushOpProps : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()
public:

	virtual float GetStrength() { return 1.0f; }
	virtual float GetDepth() { return 0.0f; }
	virtual float GetFalloff() { return 0.5f; }

	// support for this is optional, used by UI level to edit brush props via hotkeys/etc
	virtual void SetStrength(float NewStrength) { }
	virtual void SetFalloff(float NewFalloff) { }
};



class HANDYMAN_API FHandyManMeshSculptBrushOp
{
public:
	virtual ~FHandyManMeshSculptBrushOp() {}

	TWeakObjectPtr<UHandyManMeshSculptBrushOpProps> PropertySet;

	template<typename PropType> 
	PropType* GetPropertySetAs()
	{
		check(PropertySet.IsValid());
		return CastChecked<PropType>(PropertySet.Get());
	}


	TSharedPtr<FHandyManMeshSculptFalloffFunc> Falloff;
	FHandyManSculptBrushOptions CurrentOptions;

	const FHandyManMeshSculptFalloffFunc& GetFalloff() const { return *Falloff; }

	virtual void ConfigureOptions(const FHandyManSculptBrushOptions& Options)
	{
		CurrentOptions = Options;
	}

	virtual void BeginStroke(const FDynamicMesh3* Mesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& InitialVertices) {}
	virtual void EndStroke(const FDynamicMesh3* Mesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& FinalVertices) {}
	virtual void CancelStroke() {}
	virtual void ApplyStamp(const FDynamicMesh3* Mesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewValuesOut) = 0;



	//
	// overrideable Brush Op configuration things
	//

	virtual EHandyManSculptBrushOpTargetType GetBrushTargetType() const
	{
		return EHandyManSculptBrushOpTargetType::SculptMesh;
	}

	virtual bool GetAlignStampToView() const
	{
		return false;
	}

	virtual bool IgnoreZeroMovements() const
	{
		return false;
	}


	virtual bool WantsStampRegionPlane() const
	{
		return false;
	}

	virtual bool SupportsVariableSpacing() const
	{
		return false;
	}
};




class HANDYMAN_API FHandyManMeshSculptBrushOpFactory
{
public:
	virtual ~FHandyManMeshSculptBrushOpFactory() {}
	virtual TUniquePtr<FHandyManMeshSculptBrushOp> Build() = 0;
};

template<typename OpType>
class THandyManBasicMeshSculptBrushOpFactory : public FHandyManMeshSculptBrushOpFactory
{
public:
	virtual TUniquePtr<FHandyManMeshSculptBrushOp> Build() override
	{
		return MakeUnique<OpType>();
	}
};


class FHandyManLambdaMeshSculptBrushOpFactory : public FHandyManMeshSculptBrushOpFactory
{
public:
	TUniqueFunction<TUniquePtr<FHandyManMeshSculptBrushOp>(void)> BuildFunc;

	FHandyManLambdaMeshSculptBrushOpFactory()
	{
	}

	FHandyManLambdaMeshSculptBrushOpFactory(TUniqueFunction<TUniquePtr<FHandyManMeshSculptBrushOp>(void)> BuildFuncIn)
	{
		BuildFunc = MoveTemp(BuildFuncIn);
	}

	virtual TUniquePtr<FHandyManMeshSculptBrushOp> Build() override
	{
		return BuildFunc();
	}
};