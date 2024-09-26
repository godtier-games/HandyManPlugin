
#pragma once

#include "MatrixTypes.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Deformers/Kelvinlets.h"
#include "Async/ParallelFor.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Operators/HandyManMeshBrushOperators.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Tool/HandyManSculptTool.h"
#include "HandyManKelvinletBrushOp.generated.h"

// import HandyManKelvinlet types
using UE::Geometry::FPullKelvinlet;
using UE::Geometry::FLaplacianPullKelvinlet;
using UE::Geometry::FBiLaplacianPullKelvinlet;
using UE::Geometry::FSharpLaplacianPullKelvinlet;
using UE::Geometry::FSharpBiLaplacianPullKelvinlet;
using UE::Geometry::FAffineKelvinlet;
using UE::Geometry::FTwistKelvinlet;
using UE::Geometry::FPinchKelvinlet;
using UE::Geometry::FScaleKelvinlet;
using UE::Geometry::FBlendPullSharpKelvinlet;
using UE::Geometry::FBlendPullKelvinlet;
using UE::Geometry::FLaplacianTwistPullKelvinlet;
using UE::Geometry::FBiLaplacianTwistPullKelvinlet;

UCLASS()
class HANDYMAN_API UBaseHandyManKelvinletBrushOpProps : public UHandyManMeshSculptBrushOpProps
{
	GENERATED_BODY()

public:

	/** How much the mesh resists shear */
	UPROPERTY()
	float Stiffness = 1.f;

	/** How compressible the spatial region is: 1 - 2 x Poisson ratio */
	UPROPERTY()
	float Incompressiblity = 1.f;

	/** Integration steps*/
	UPROPERTY()
	int32 BrushSteps = 3;

	virtual float GetStiffness() { return Stiffness; }
	virtual float GetIncompressiblity() { return Incompressiblity; }
	virtual int32 GetNumSteps() { return BrushSteps; }

};





class FBaseHandyManKelvinletBrushOp : public FHandyManMeshSculptBrushOp
{
public:
	
	void SetBaseProperties(const FDynamicMesh3* SrcMesh, const FHandyManSculptBrushStamp& Stamp)
	{
		// pointer to mesh
		Mesh = SrcMesh;
		UBaseHandyManKelvinletBrushOpProps* BaseHandyManKelvinletBrushOpProps = GetPropertySetAs<UBaseHandyManKelvinletBrushOpProps>();

		float Stiffness = BaseHandyManKelvinletBrushOpProps->GetStiffness();
		float Incompressibility = BaseHandyManKelvinletBrushOpProps->GetIncompressiblity();
		int NumIntSteps = BaseHandyManKelvinletBrushOpProps->GetNumSteps();

		Mu = FMath::Max(Stiffness, 0.f);
		Nu = FMath::Clamp(0.5f * (1.f - 2.f * Incompressibility), 0.f, 0.5f);
		
		Size = FMath::Max(Stamp.Radius, 0.01);

		NumSteps = NumIntSteps;
	}


	template <typename HandyManKelvinletType>
	void ApplyHandyManKelvinlet(const HandyManKelvinletType& HandyManKelvinlet, const FFrame3d& LocalFrame, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) const 
	{

		if (NumSteps == 0)
		{
			DisplaceHandyManKelvinlet(HandyManKelvinlet, LocalFrame, Vertices, NewPositionsOut);
		}
		else
		{
			IntegrateHandyManKelvinlet(HandyManKelvinlet, LocalFrame, Vertices, NewPositionsOut, IntegrationTime, NumSteps);
		}
	}



	virtual void ApplyStamp(const FDynamicMesh3*SrcMesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override {}

protected:

	void ApplyFalloff(const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) const
	{
		int32 NumV = Vertices.Num();
		bool bParallel = true;
		ParallelFor(NumV, [&Stamp, &Vertices, &NewPositionsOut, this](int32 k)
			{
				int32 VertIdx = Vertices[k];
				FVector3d OrigPos = Mesh->GetVertex(VertIdx);

				double Falloff = GetFalloff().Evaluate(Stamp, OrigPos);

				double Alpha = FMath::Clamp(Falloff, 0., 1.);

				NewPositionsOut[k] = Alpha * NewPositionsOut[k] + (1. - Alpha) * OrigPos;
			}, bParallel);
	}


	// NB: this just moves the verts, but doesn't update the normal.  The HandyManKelvinlets will have to be extended if we want
	// to do the Jacobian Transpose operation on the normals - but for now, we should just rebuild the normals after the brush
	template <typename HandyManKelvinletType>
	void DisplaceHandyManKelvinlet(const HandyManKelvinletType& HandyManKelvinlet, const FFrame3d& LocalFrame, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) const
	{
		int32 NumV = Vertices.Num();
		NewPositionsOut.SetNum(NumV, EAllowShrinking::No);

		const bool bForceSingleThread = false;

		ParallelFor(NumV, [&HandyManKelvinlet, &Vertices, &NewPositionsOut, &LocalFrame, this](int32 k)
			{
				int VertIdx = Vertices[k];
				FVector3d Pos = Mesh->GetVertex(VertIdx);
				Pos = LocalFrame.ToFramePoint(Pos);

				Pos = HandyManKelvinlet.Evaluate(Pos) + Pos;

				// Update the position in the ROI Array
				NewPositionsOut[k] = LocalFrame.FromFramePoint(Pos);
			}
		, bForceSingleThread);

	}

	template <typename HandyManKelvinletType>
	void IntegrateHandyManKelvinlet(const HandyManKelvinletType& HandyManKelvinlet, const FFrame3d& LocalFrame, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut, const double Dt, const int32 Steps) const
	{
		int NumV = Vertices.Num();
		NewPositionsOut.SetNum(NumV, EAllowShrinking::No);

		const bool bForceSingleThread = false;

		ParallelFor(NumV, [&HandyManKelvinlet, &Vertices, &NewPositionsOut, &LocalFrame, Dt, Steps, this](int32 k)
			{
				int VertIdx = Vertices[k];
				FVector3d Pos = LocalFrame.ToFramePoint(Mesh->GetVertex(VertIdx));

				double TimeScale = 1. / (Steps);
				// move with several time steps
				for (int i = 0; i < Steps; ++i)
				{
					// the position after deformation
					Pos = HandyManKelvinlet.IntegrateRK3(Pos, Dt * TimeScale);
				}
				// Update the position in the ROI Array
				NewPositionsOut[k] = LocalFrame.FromFramePoint(Pos);
			}, bForceSingleThread);


	}

protected:


	// physical properties.
	float Mu;
	float Nu;
	// model regularization parameter
	float Size;
	
	// integration parameters
	int32 NumSteps;
	double IntegrationTime = 1.;
	
	const FDynamicMesh3* Mesh;
};





UCLASS()
class HANDYMAN_API UScaleHandyManKelvinletBrushOpProps : public  UBaseHandyManKelvinletBrushOpProps
{
	GENERATED_BODY()

public:
	/** Strength of the Brush */
	UPROPERTY(EditAnywhere, Category = KelvinScaleBrush, meta = (DisplayName = "Strength", UIMin = "0.0", UIMax = "10.", ClampMin = "0.0", ClampMax = "10."))
	float Strength = 0.5;

	/** Amount of falloff to apply */
	UPROPERTY(EditAnywhere, Category = KelvinScaleBrush, meta = (DisplayName = "Falloff", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Falloff = 0.5;


	virtual float GetStrength() override { return Strength; }
	virtual float GetFalloff() override { return Falloff; }
};

class FScaleHandyManKelvinletBrushOp : public  FBaseHandyManKelvinletBrushOp
{

public:
	
	virtual void ApplyStamp(const FDynamicMesh3* SrcMesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override
	{
		SetBaseProperties(SrcMesh, Stamp);

		float Strength = GetPropertySetAs<UScaleHandyManKelvinletBrushOpProps>()->GetStrength();

		float Speed = Strength * 0.025 * FMath::Sqrt(Stamp.Radius) * Stamp.Direction;
		FScaleKelvinlet ScaleHandyManKelvinlet(Speed, 0.35 * Size, Mu, Nu);

		ApplyHandyManKelvinlet(ScaleHandyManKelvinlet, Stamp.LocalFrame, Vertices, NewPositionsOut);

		ApplyFalloff(Stamp, Vertices, NewPositionsOut);
	}

};

UCLASS()
class HANDYMAN_API UPullHandyManKelvinletBrushOpProps : public UBaseHandyManKelvinletBrushOpProps
{
	GENERATED_BODY()
public:
	
	/** Amount of falloff to apply */
	UPROPERTY(EditAnywhere, Category = KelvinGrabBrush, meta = (DisplayName = "Falloff", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Falloff = 0.1;

	/** Depth of Brush into surface along view ray */
	UPROPERTY(EditAnywhere, Category = KelvinGrabBrush, meta = (UIMin = "-0.5", UIMax = "0.5", ClampMin = "-1.0", ClampMax = "1.0"))
	float Depth = 0;

	// these get routed into the Stamp
	virtual float GetFalloff() override { return Falloff; }
	virtual float GetDepth() override { return Depth; }
};

class FPullHandyManKelvinletBrushOp : public  FBaseHandyManKelvinletBrushOp
{

public:

	virtual void ApplyStamp(const FDynamicMesh3* SrcMesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override
	{
		SetBaseProperties(SrcMesh, Stamp);

		// compute the displacement vector in the local frame of the stamp
		FVector3d Force = Stamp.LocalFrame.ToFrameVector(Stamp.LocalFrame.Origin - Stamp.PrevLocalFrame.Origin);
		Size *= 0.6;

		FLaplacianPullKelvinlet LaplacianPullHandyManKelvinlet(Force, Size, Mu, Nu);
		FBiLaplacianPullKelvinlet BiLaplacianPullHandyManKelvinlet(Force, Size, Mu, Nu);

		const double Alpha = Stamp.Falloff;
		// Lerp between a broad and a narrow HandyManKelvinlet based on the fall-off 
		FBlendPullKelvinlet BlendPullHandyManKelvinlet(BiLaplacianPullHandyManKelvinlet, LaplacianPullHandyManKelvinlet, Alpha);
		ApplyHandyManKelvinlet(BlendPullHandyManKelvinlet, Stamp.LocalFrame, Vertices, NewPositionsOut);

		ApplyFalloff(Stamp, Vertices, NewPositionsOut);
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

UCLASS()
class HANDYMAN_API USharpPullHandyManKelvinletBrushOpProps : public UBaseHandyManKelvinletBrushOpProps
{
	GENERATED_BODY()
public:

	/** Amount of falloff to apply */
	UPROPERTY(EditAnywhere, Category = KelvinSharpGrabBrush, meta = (DisplayName = "Falloff", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Falloff = 0.5;

	/** Depth of Brush into surface along view ray */
	UPROPERTY(EditAnywhere, Category = KelvinSharpGrabBrush, meta = (UIMin = "-0.5", UIMax = "0.5", ClampMin = "-1.0", ClampMax = "1.0"))
	float Depth = 0;

	// these get routed into the Stamp
	virtual float GetFalloff() override { return Falloff; }
	virtual float GetDepth() override { return Depth; }
};

class FSharpPullHandyManKelvinletBrushOp : public  FBaseHandyManKelvinletBrushOp
{

public:

	virtual void ApplyStamp(const FDynamicMesh3* SrcMesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override
	{
		SetBaseProperties(SrcMesh, Stamp);

		// compute the displacement vector in the local frame of the stamp
		FVector3d Force = Stamp.LocalFrame.ToFrameVector(Stamp.LocalFrame.Origin - Stamp.PrevLocalFrame.Origin);

		FSharpLaplacianPullKelvinlet SharpLaplacianPullHandyManKelvinlet(Force, Size, Mu, Nu);
		FSharpBiLaplacianPullKelvinlet SharpBiLaplacianPullHandyManKelvinlet(Force, Size, Mu, Nu);

		const double Alpha = Stamp.Falloff;
		// Lerp between a broad and a narrow HandyManKelvinlet based on the fall-off 
		FBlendPullSharpKelvinlet BlendPullSharpHandyManKelvinlet(SharpBiLaplacianPullHandyManKelvinlet, SharpLaplacianPullHandyManKelvinlet, Alpha);

		ApplyHandyManKelvinlet(BlendPullSharpHandyManKelvinlet, Stamp.LocalFrame, Vertices, NewPositionsOut);

		ApplyFalloff(Stamp, Vertices, NewPositionsOut);
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

class FLaplacianPullHandyManKelvinletBrushOp : public FBaseHandyManKelvinletBrushOp
{
public:

	virtual void ApplyStamp(const FDynamicMesh3* SrcMesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override
	{
		SetBaseProperties(SrcMesh, Stamp);

		// compute the displacement vector in the local frame of the stamp
		FVector3d Force = Stamp.LocalFrame.ToFrameVector(Stamp.LocalFrame.Origin - Stamp.PrevLocalFrame.Origin);

		FLaplacianPullKelvinlet PullHandyManKelvinlet(Force, Size, Mu, Nu);
		ApplyHandyManKelvinlet(PullHandyManKelvinlet, Stamp.LocalFrame, Vertices, NewPositionsOut);

		ApplyFalloff(Stamp, Vertices, NewPositionsOut);
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

class FBiLaplacianPullHandyManKelvinletBrushOp : public FBaseHandyManKelvinletBrushOp
{
public:

	virtual void ApplyStamp(const FDynamicMesh3* SrcMesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override
	{
		SetBaseProperties(SrcMesh, Stamp);

		// compute the displacement vector in the local frame of the stamp
		FVector3d Force = Stamp.LocalFrame.ToFrameVector(Stamp.LocalFrame.Origin - Stamp.PrevLocalFrame.Origin);

		FBiLaplacianPullKelvinlet PullHandyManKelvinlet(Force, Size, Mu, Nu);
		ApplyHandyManKelvinlet(PullHandyManKelvinlet, Stamp.LocalFrame, Vertices, NewPositionsOut);

		ApplyFalloff(Stamp, Vertices, NewPositionsOut);
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

UCLASS()
class HANDYMAN_API UTwistHandyManKelvinletBrushOpProps : public UBaseHandyManKelvinletBrushOpProps
{
	GENERATED_BODY()

public:
	/** Twisting strength of the Brush */
	UPROPERTY(EditAnywhere, Category = KelvinTwistBrush, meta = (DisplayName = "Strength", UIMin = "0.0", UIMax = "10.", ClampMin = "0.0", ClampMax = "10."))
	float Strength = 0.5;

	/** Amount of falloff to apply */
	UPROPERTY(EditAnywhere, Category = KelvinTwistBrush, meta = (DisplayName = "Falloff", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float Falloff = 0.5;


	virtual float GetStrength() override { return Strength; }
	virtual float GetFalloff() override { return Falloff; }
};

class FTwistHandyManKelvinletBrushOp : public FBaseHandyManKelvinletBrushOp
{
public:


	virtual void ApplyStamp(const FDynamicMesh3* SrcMesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override
	{
		SetBaseProperties(SrcMesh, Stamp);

		float Strength = GetPropertySetAs<UTwistHandyManKelvinletBrushOpProps>()->GetStrength();

		float Speed = Strength * Stamp.Direction;
		
		// In the local frame, twist axis is in "z" direction 
		FVector3d LocalTwistAxis = FVector3d(0., 0., Speed);

		FTwistKelvinlet TwistHandyManKelvinlet(LocalTwistAxis, Size, Mu, Nu);
		ApplyHandyManKelvinlet(TwistHandyManKelvinlet, Stamp.LocalFrame, Vertices, NewPositionsOut);

		ApplyFalloff(Stamp, Vertices, NewPositionsOut);
	}

	virtual EHandyManSculptBrushOpTargetType GetBrushTargetType() const override
	{
		return EHandyManSculptBrushOpTargetType::ActivePlane;
	}
};

class FPinchHandyManKelvinletBrushOp : public FBaseHandyManKelvinletBrushOp
{
public:
	virtual void ApplyStamp(const FDynamicMesh3* SrcMesh, const FHandyManSculptBrushStamp& Stamp, const TArray<int32>& Vertices, TArray<FVector3d>& NewPositionsOut) override
	{
		// is this really the vector we want?
		// compute the displacement vector in the local frame of the stamp
		FVector3d Dir = Stamp.LocalFrame.ToFrameVector(Stamp.LocalFrame.Origin - Stamp.PrevLocalFrame.Origin);

		FMatrix3d ForceMatrix = UE::Geometry::CrossProductMatrix(Dir);
		// make symmetric
		ForceMatrix.Row0[1] = -ForceMatrix.Row0[1];
		ForceMatrix.Row0[2] = -ForceMatrix.Row0[2];
		ForceMatrix.Row1[2] = -ForceMatrix.Row1[2];
		FPinchKelvinlet PinchHandyManKelvinlet(ForceMatrix, Size, Mu, Nu);
		ApplyHandyManKelvinlet(PinchHandyManKelvinlet, Stamp.LocalFrame, Vertices, NewPositionsOut);

		ApplyFalloff(Stamp, Vertices, NewPositionsOut);
	}
};


/**--------------------- Glue Layer Currently Used in the DynamicMeshSculptTool:   --------------------------
* 
* We should be able to remove these once the dynamic mesh sculpt tool starts using the new brush infrastructure ( used by the MeshVertexSculptTool)
*/

enum class EHandyManKelvinletBrushMode
{
	ScaleHandyManKelvinlet,
	PinchHandyManKelvinlet,
	TwistHandyManKelvinlet,
	PullHandyManKelvinlet,
	LaplacianPullHandyManKelvinlet,
	BiLaplacianPullHandyManKelvinlet,
	BiLaplacianTwistPullHandyManKelvinlet,
	LaplacianTwistPullHandyManKelvinlet,
	SharpPullHandyManKelvinlet
};

class FHandyManKelvinletBrushOp
{
public:


	struct FHandyManKelvinletBrushOpProperties
	{
		FHandyManKelvinletBrushOpProperties(const EHandyManKelvinletBrushMode& BrushMode, const UHandyManKelvinBrushProperties& Properties, double BrushRadius, double BrushFalloff) 
			: Mode(BrushMode)
			, Direction(1.0, 0.0, 0.0)
		{
			Speed = 0.;
			FallOff = BrushFalloff;
			Mu = FMath::Max(Properties.Stiffness, 0.f);
			Nu = FMath::Clamp(0.5f * (1.f - 2.f * Properties.Incompressiblity), 0.f, 0.5f);
			Size = FMath::Max(BrushRadius * Properties.FallOffDistance, 0.f);
			NumSteps = Properties.BrushSteps;
		}


		EHandyManKelvinletBrushMode Mode;
		FVector Direction;

		double Speed;   // Optionally used
		double FallOff; // Optionally used
		double Mu; // Shear Modulus
		double Nu; // Poisson ratio

		// regularization parameter
		double Size;

		int NumSteps;

	};

	FHandyManKelvinletBrushOp(const FDynamicMesh3& DynamicMesh) :
		Mesh(&DynamicMesh)
	{}

	void ExtractTransform(const FMatrix& WorldToBrush)
	{
		// Extract the parts of the transform and account for the vector * matrix  format of FMatrix by transposing.

		// Transpose of the 3x3 part
		WorldToBrushMat.Row0[0] = WorldToBrush.M[0][0];
		WorldToBrushMat.Row0[1] = WorldToBrush.M[0][1];
		WorldToBrushMat.Row0[2] = WorldToBrush.M[0][2];

		WorldToBrushMat.Row1[0] = WorldToBrush.M[1][0];
		WorldToBrushMat.Row1[1] = WorldToBrush.M[1][1];
		WorldToBrushMat.Row1[2] = WorldToBrush.M[1][2];

		WorldToBrushMat.Row2[0] = WorldToBrush.M[2][0];
		WorldToBrushMat.Row2[1] = WorldToBrush.M[2][1];
		WorldToBrushMat.Row2[2] = WorldToBrush.M[2][2];

		Translation[0] = WorldToBrush.M[3][0];
		Translation[1] = WorldToBrush.M[3][1];
		Translation[2] = WorldToBrush.M[3][2];

		// The matrix should be unitary (det +/- 1) but want this to work with 
		// more general input if needed so just make sure we can invert the matrix
		check(FMath::Abs(WorldToBrush.Determinant()) > 1.e-4);

		BrushToWorldMat = WorldToBrushMat.Inverse();

	};

	~FHandyManKelvinletBrushOp() {}

public:
	const FDynamicMesh3* Mesh;


	double TimeStep = 1.0;
	double NumSteps = 0.0;

	// To be applied as WorlToBrushMat * v + Trans
	// Note: could use FTransform3d in TransformTypes.h
	FMatrix3d WorldToBrushMat;
	FMatrix3d BrushToWorldMat;
	FVector3d Translation;

public:



	void ApplyBrush(const FHandyManKelvinletBrushOpProperties& Properties, const FMatrix& WorldToBrush, const TArray<int>& VertRIO, TArray<FVector3d>& ROIPositionBuffer)
	{

		ExtractTransform(WorldToBrush);

		NumSteps = Properties.NumSteps;

		switch (Properties.Mode)
		{

		case EHandyManKelvinletBrushMode::ScaleHandyManKelvinlet:
		{
			double Scale = Properties.Direction.X;
			FScaleKelvinlet ScaleHandyManKelvinlet(Scale, Properties.Size, Properties.Mu, Properties.Nu);
			ApplyHandyManKelvinlet(ScaleHandyManKelvinlet, VertRIO, ROIPositionBuffer, TimeStep, NumSteps);
			break;
		}
		case EHandyManKelvinletBrushMode::PullHandyManKelvinlet:
		{
			FVector Dir = Properties.Direction;
			FVector3d Force(Dir.X, Dir.Y, Dir.Z);

			FLaplacianPullKelvinlet LaplacianPullHandyManKelvinlet(Force, Properties.Size, Properties.Mu, Properties.Nu);
			FBiLaplacianPullKelvinlet BiLaplacianPullHandyManKelvinlet(Force, Properties.Size, Properties.Mu, Properties.Nu);

			const double Alpha = Properties.FallOff;
			// Lerp between a broad and a narrow HandyManKelvinlet based on the fall-off 
			FBlendPullKelvinlet BlendPullHandyManKelvinlet(BiLaplacianPullHandyManKelvinlet, LaplacianPullHandyManKelvinlet, Alpha);
			ApplyHandyManKelvinlet(BlendPullHandyManKelvinlet, VertRIO, ROIPositionBuffer, TimeStep, NumSteps);
			break;
		}
		case EHandyManKelvinletBrushMode::SharpPullHandyManKelvinlet:
		{
			FVector Dir = Properties.Direction;
			FVector3d Force(Dir.X, Dir.Y, Dir.Z);

			FSharpLaplacianPullKelvinlet SharpLaplacianPullHandyManKelvinlet(Force, Properties.Size, Properties.Mu, Properties.Nu);
			FSharpBiLaplacianPullKelvinlet SharpBiLaplacianPullHandyManKelvinlet(Force, Properties.Size, Properties.Mu, Properties.Nu);

			const double Alpha = Properties.FallOff;
			// Lerp between a broad and a narrow HandyManKelvinlet based on the fall-off 
			FBlendPullSharpKelvinlet BlendPullSharpHandyManKelvinlet(SharpBiLaplacianPullHandyManKelvinlet, SharpLaplacianPullHandyManKelvinlet, Alpha);
			ApplyHandyManKelvinlet(BlendPullSharpHandyManKelvinlet, VertRIO, ROIPositionBuffer, TimeStep, NumSteps);
			break;
		}
		case EHandyManKelvinletBrushMode::LaplacianPullHandyManKelvinlet:
		{
			FVector Dir = Properties.Direction;
			FVector3d Force(Dir.X, Dir.Y, Dir.Z);
			FLaplacianPullKelvinlet PullHandyManKelvinlet(Force, Properties.Size, Properties.Mu, Properties.Nu);
			ApplyHandyManKelvinlet(PullHandyManKelvinlet, VertRIO, ROIPositionBuffer, TimeStep, NumSteps);
			break;
		}
		case EHandyManKelvinletBrushMode::BiLaplacianPullHandyManKelvinlet:
		{
			FVector Dir = Properties.Direction;
			FVector3d Force(Dir.X, Dir.Y, Dir.Z);
			FBiLaplacianPullKelvinlet PullHandyManKelvinlet(Force, Properties.Size, Properties.Mu, Properties.Nu);
			ApplyHandyManKelvinlet(PullHandyManKelvinlet, VertRIO, ROIPositionBuffer, TimeStep, NumSteps);
			break;
		}
		case EHandyManKelvinletBrushMode::TwistHandyManKelvinlet:
		{
			FVector TwistAxis = Properties.Direction;
			FTwistKelvinlet TwistHandyManKelvinlet((FVector3d)TwistAxis, Properties.Size, Properties.Mu, Properties.Nu);
			ApplyHandyManKelvinlet(TwistHandyManKelvinlet, VertRIO, ROIPositionBuffer, TimeStep, NumSteps);
			break;
		}
		case EHandyManKelvinletBrushMode::PinchHandyManKelvinlet:
		{
			FVector Dir = Properties.Direction;

			FMatrix3d ForceMatrix = UE::Geometry::CrossProductMatrix(FVector3d(Dir.X, Dir.Y, Dir.Z));
			// make symmetric
			ForceMatrix.Row0[1] = -ForceMatrix.Row0[1];
			ForceMatrix.Row0[2] = -ForceMatrix.Row0[2];
			ForceMatrix.Row1[2] = -ForceMatrix.Row1[2];
			FPinchKelvinlet PinchHandyManKelvinlet(ForceMatrix, Properties.Size, Properties.Mu, Properties.Nu);
			ApplyHandyManKelvinlet(PinchHandyManKelvinlet, VertRIO, ROIPositionBuffer, TimeStep, NumSteps);
			break;
		}
		case EHandyManKelvinletBrushMode::LaplacianTwistPullHandyManKelvinlet:
		{
			FVector3d TwistAxis = (FVector3d)Properties.Direction;
			UE::Geometry::Normalize(TwistAxis);
			TwistAxis *= Properties.Speed;
			FTwistKelvinlet TwistHandyManKelvinlet(TwistAxis, Properties.Size, Properties.Mu, Properties.Nu);

			FVector Dir = Properties.Direction;
			FVector3d Force(Dir.X, Dir.Y, Dir.Z);
			FLaplacianPullKelvinlet PullHandyManKelvinlet(Force, Properties.Size, Properties.Mu, Properties.Nu);

			FLaplacianTwistPullKelvinlet TwistPullHandyManKelvinlet(TwistHandyManKelvinlet, PullHandyManKelvinlet, 0.5);
			ApplyHandyManKelvinlet(TwistPullHandyManKelvinlet, VertRIO, ROIPositionBuffer, TimeStep, NumSteps);
			break;

		}
		default:
			check(0);
		}
	}


	// NB: this just moves the verts, but doesn't update the normal.  The HandyManKelvinlets will have to be extended if we want
	// to do the Jacobian Transpose operation on the normals - but for now, we should just rebuild the normals after the brush
	template <typename HandyManKelvinletType>
	void DisplaceHandyManKelvinlet(const HandyManKelvinletType& HandyManKelvinlet, const TArray<int>& VertexROI, TArray<FVector3d>& ROIPositionBuffer)
	{
		int NumV = VertexROI.Num();
		ROIPositionBuffer.SetNum(NumV, EAllowShrinking::No);

		const bool bForceSingleThread = false;

		ParallelFor(NumV, [&HandyManKelvinlet, &VertexROI, &ROIPositionBuffer, this](int k)

		{
			int VertIdx = VertexROI[k];
			FVector3d Pos = Mesh->GetVertex(VertIdx);
			Pos = XForm(Pos);

			Pos = HandyManKelvinlet.Evaluate(Pos) + Pos;

			// Update the position in the ROI Array
			ROIPositionBuffer[k] = InvXForm(Pos);
		}
		, bForceSingleThread);

	}

	template <typename HandyManKelvinletType>
	void IntegrateHandyManKelvinlet(const HandyManKelvinletType& HandyManKelvinlet, const TArray<int>& VertexROI, TArray<FVector3d>& ROIPositionBuffer, const double Dt, const int Steps)
	{
		int NumV = VertexROI.Num();
		ROIPositionBuffer.SetNum(NumV, EAllowShrinking::No);

		const bool bForceSingleThread = false;

		ParallelFor(NumV, [&HandyManKelvinlet, &VertexROI, &ROIPositionBuffer, Dt, Steps, this](int k)
		{
			int VertIdx = VertexROI[k];
			FVector3d Pos = XForm(Mesh->GetVertex(VertIdx));

			double TimeScale = 1. / (Steps);
			// move with several time steps
			for (int i = 0; i < Steps; ++i)
			{
				// the position after deformation
				Pos = HandyManKelvinlet.IntegrateRK3(Pos, Dt * TimeScale);
			}
			// Update the position in the ROI Array
			ROIPositionBuffer[k] = InvXForm(Pos);
		}, bForceSingleThread);


	}

	template <typename HandyManKelvinletType>
	void ApplyHandyManKelvinlet(const HandyManKelvinletType& HandyManKelvinlet, const TArray<int>& VertexROI, TArray<FVector3d>& ROIPositionBuffer, const double Dt, const int NumIntegrationSteps)
	{

		if (NumIntegrationSteps == 0)
		{
			DisplaceHandyManKelvinlet(HandyManKelvinlet, VertexROI, ROIPositionBuffer);
		}
		else
		{
			IntegrateHandyManKelvinlet(HandyManKelvinlet, VertexROI, ROIPositionBuffer, Dt, NumIntegrationSteps);
		}
	}

private:

	// apply the transform.
	FVector3d XForm(const FVector3d& Pos) const
	{
		return WorldToBrushMat * Pos + Translation;
	}

	// apply the inverse transform.
	FVector3d InvXForm(const FVector3d& Pos) const
	{
		return BrushToWorldMat * (Pos - Translation);
	}

};