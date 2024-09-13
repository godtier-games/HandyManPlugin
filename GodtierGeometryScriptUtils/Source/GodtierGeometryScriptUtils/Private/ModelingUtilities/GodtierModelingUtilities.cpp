// Fill out your copyright notice in the Description page of Project Settings.


#include "ModelingUtilities/GodtierModelingUtilities.h"

#include "DynamicMeshActor.h"
#include "DynamicMeshEditor.h"
#include "MatrixTypes.h"
#include "UDynamicMesh.h"
#include "Components/SplineComponent.h"
#include "CurveOps/TriangulateCurvesOp.h"
#include "DynamicMesh/MeshTransforms.h"
#include "Engine/StaticMeshActor.h"
#include "Generators/SweepGenerator.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshModelingFunctions.h"
#include "GeometryScript/MeshRepairFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "GeometryScript/MeshSelectionQueryFunctions.h"
#include "GeometryScript/MeshTransformFunctions.h"
#include "GeometryScript/PolyPathFunctions.h"
#include "Selections/GeometrySelection.h"

using namespace UE::Geometry;

#pragma region MeshPrimitiveFunctions
namespace GodtierMeshPrimitiveFunctions
{
	static void ApplyPrimitiveOptionsToMesh(
	FDynamicMesh3& Mesh, const FTransform& Transform, 
	FGeometryScriptPrimitiveOptions PrimitiveOptions, 
	FVector3d PreTranslate = FVector3d::Zero(),
	TOptional<FQuaterniond> PreRotate = TOptional<FQuaterniond>())
	{
		bool bHasTranslate = PreTranslate.SquaredLength() > 0;
		if (PreRotate.IsSet())
		{
			FFrame3d Frame(PreTranslate, *PreRotate);
			MeshTransforms::FrameCoordsToWorld(Mesh, Frame);
		}
		else if (bHasTranslate)
		{
			MeshTransforms::Translate(Mesh, PreTranslate);
		}

		MeshTransforms::ApplyTransform(Mesh, (FTransformSRT3d)Transform, true);
		if (PrimitiveOptions.PolygroupMode == EGeometryScriptPrimitivePolygroupMode::SingleGroup)
		{
			for (int32 tid : Mesh.TriangleIndicesItr())
			{
				Mesh.SetTriangleGroup(tid, 0);
			}
		}
		if (PrimitiveOptions.bFlipOrientation)
		{
			Mesh.ReverseOrientation(true);
			if (Mesh.HasAttributes())
			{
				FDynamicMeshNormalOverlay* Normals = Mesh.Attributes()->PrimaryNormals();
				for (int elemid : Normals->ElementIndicesItr())
				{
					Normals->SetElement(elemid, -Normals->GetElement(elemid));
				}
			}
		}
	}
	
	static void AppendPrimitive(
	UDynamicMesh* TargetMesh, 
	FMeshShapeGenerator* Generator, 
	FTransform Transform, 
	FGeometryScriptPrimitiveOptions PrimitiveOptions,
	FVector3d PreTranslate = FVector3d::Zero(),
	TOptional<FQuaterniond> PreRotate = TOptional<FQuaterniond>())
	{
		if (TargetMesh->IsEmpty())
		{
			TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh)
			{
				EditMesh.Copy(Generator);
				ApplyPrimitiveOptionsToMesh(EditMesh, Transform, PrimitiveOptions, PreTranslate, PreRotate);
			}, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);
		}
		else
		{
			FDynamicMesh3 TempMesh(Generator);
			ApplyPrimitiveOptionsToMesh(TempMesh, Transform, PrimitiveOptions, PreTranslate, PreRotate);
			TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh)
			{
				FMeshIndexMappings TmpMappings;
				FDynamicMeshEditor Editor(&EditMesh);
				Editor.AppendMesh(&TempMesh, TmpMappings);
			}, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);
		}
	}
}
#pragma endregion 

#define LOCTEXT_NAMESPACE "GodtierModelingUtilities"

UGodtierModelingUtilities::UGodtierModelingUtilities(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UDynamicMesh* UGodtierModelingUtilities::SweepGeometryAlongSpline(FSweepOptions SweepOptions, const ESplineCoordinateSpace::Type Space, UGeometryScriptDebug* Debug)
{
	auto TargetMesh = SweepOptions.TargetMesh;
	auto Spline = SweepOptions.Spline;
	
	TArray<FVector2D> SweepShapeVertices;
	TArray<FTransform> SweepPath;
	TArray<double> SweepFrameTimes;

	
	
	if (TargetMesh == nullptr)
	{
		UE::Geometry::AppendError(Debug, EGeometryScriptErrorType::InvalidInputs, LOCTEXT("AppendSweepPolyline_NullMesh", "AppendSweepPolyline: TargetMesh is Null"));
		return TargetMesh;
	}

	if (SweepOptions.bResetTargetMesh)
	{
		TargetMesh->Reset();
	}
	
	if (Spline->GetNumberOfSplinePoints() < 2)
	{
		UE::Geometry::AppendError(Debug, EGeometryScriptErrorType::InvalidInputs, LOCTEXT("AppendSweepPolyline_InvalidSweepPath", "AppendSweepPolyline: SweepPath array requires at least 2 positions"));
		return TargetMesh;
	}
	
	switch (SweepOptions.ShapeType)
	{
		case ESweepShapeType::Circle:
			{
				auto GeometryPath = UGeometryScriptLibrary_PolyPathFunctions::CreateCirclePath3D(FTransform::Identity, SweepOptions.ShapeRadius, SweepOptions.ShapeSegments);
				for (int i = 0; i < GeometryPath.Path->Num(); i++)
				{
					SweepShapeVertices.Add(FVector2D(GeometryPath.Path->GetData()[i].X, GeometryPath.Path->GetData()[i].Y));
				}
				SweepShapeVertices.Emplace(*GeometryPath.Path->GetData());
				break;
			}
		case ESweepShapeType::Triangle:
			{
				auto GeometryPath = UGeometryScriptLibrary_PolyPathFunctions::CreateCirclePath3D(FTransform::Identity, SweepOptions.ShapeRadius, 3);
				for (int i = 0; i < GeometryPath.Path->Num(); i++)
				{
					SweepShapeVertices.Add(FVector2D(GeometryPath.Path->GetData()[i].X, GeometryPath.Path->GetData()[i].Y));
				}
				break;
			}
		case ESweepShapeType::Box:
			{
				SweepShapeVertices.Add(FVector2D(0.0, -SweepOptions.ShapeDimensions.Y));
				SweepShapeVertices.Add(FVector2D(SweepOptions.ShapeDimensions.X, -SweepOptions.ShapeDimensions.Y));
				SweepShapeVertices.Add(FVector2D(SweepOptions.ShapeDimensions.X , SweepOptions.ShapeDimensions.Y));
				SweepShapeVertices.Add(FVector2D(0.f, SweepOptions.ShapeDimensions.Y));

			}
		case ESweepShapeType::Custom:
			{
				FGeometryScriptPolyPath GeometryPath;
				FGeometryScriptSplineSamplingOptions SamplingOptions;
				SamplingOptions.CoordinateSpace = Space;
				SamplingOptions.NumSamples = FMath::Clamp(SweepOptions.ShapeSegments, 4, MAX_int8);
			    UGeometryScriptLibrary_PolyPathFunctions::ConvertSplineToPolyPath(SweepOptions.CustomProfile, GeometryPath, SamplingOptions);
				
				for (int i = 0; i < GeometryPath.Path->Num(); i++)
				{
					SweepShapeVertices.Add(FVector2D(GeometryPath.Path->GetData()[i].X, GeometryPath.Path->GetData()[i].Y));
				}
				break;
			}
	}
	
	if (SweepShapeVertices.Num() < 2)
	{
		UE::Geometry::AppendError(Debug, EGeometryScriptErrorType::InvalidInputs, LOCTEXT("AppendSweepPolyline_InvalidPolygon", "AppendSweepPolyline: Polyline array requires at least 2 positions"));
		return TargetMesh;
	}

	bool bShouldResample = false;
	for (int i = 0; i < Spline->GetNumberOfSplinePoints(); i++)
	{
		if (Spline->GetSplinePointType(i) == ESplinePointType::CurveCustomTangent || Spline->GetSplinePointType(i) == ESplinePointType::Curve || SweepOptions.bResampleCurve )
		{
			bShouldResample = true;
			break;
		}
	}

	if (bShouldResample)
	{
		FGeometryScriptSplineSamplingOptions SampleOptions;
		SampleOptions.CoordinateSpace = Space;
		SampleOptions.NumSamples = SweepOptions.SampleSize;
		//SampleOptions.ErrorTolerance = 1.0f;
		UGeometryScriptLibrary_PolyPathFunctions::SampleSplineToTransforms(Spline, SweepPath, SweepFrameTimes, SampleOptions, FTransform::Identity);
		
		/*TArray<FVector> Vertices;
		Spline->ConvertSplineToPolyLine(ESplineCoordinateSpace::World, 1, Vertices);

		for (const auto& Point : Vertices)
		{
			SweepPath.Add(FTransform(Point));
		}*/

	}
	else
	{
		for (int i = 0; i < Spline->GetNumberOfSplinePoints(); i++)
		{
			SweepPath.Add(Spline->GetTransformAtSplinePoint(i, Space));
		}
	}


	
	if (SweepPath.Num() < 2)
	{
		UE::Geometry::AppendError(Debug, EGeometryScriptErrorType::InvalidInputs, LOCTEXT("AppendSweepPolyline_InvalidSweepPath", "AppendSweepPolyline: SweepPath array requires at least 2 positions"));
		return TargetMesh;
	}

	float RotationInDegrees = SweepOptions.RotationAngleDeg;
	if (SweepOptions.ShapeType == ESweepShapeType::Box)
	{
		RotationInDegrees = 90;
	}

	if (SweepOptions.ShapeType == ESweepShapeType::Triangle)
	{
		RotationInDegrees = 30;
	}

	FMatrix2d Rotation2D = FMatrix2d::RotationDeg(-RotationInDegrees);
	FGeneralizedCylinderGenerator SweepGen;
	for (const FVector2D& Point : SweepShapeVertices)
	{
		SweepGen.CrossSection.AppendVertex(Rotation2D * FVector2d(Point.X, Point.Y));
	}

	for (const FTransform& SweepXForm : SweepPath)
	{
		
		SweepGen.Path.Add(SweepXForm.GetLocation());
		FQuaterniond Rotation(SweepXForm.GetRotation());
		SweepGen.PathFrames.Add(
			FFrame3d(SweepXForm.GetLocation(), Rotation.AxisY(), Rotation.AxisZ(), Rotation.AxisX())
		);
		FVector3d Scale = SweepXForm.GetScale3D();
		SweepGen.PathScales.Add(FVector2d(Scale.Y, Scale.Z));
	}

	SweepGen.bProfileCurveIsClosed = true;
	SweepGen.bLoop = false;
	/*if (PolylineTexParamU.Num() > 0)
	{
		SweepGen.CrossSectionTexCoord = PolylineTexParamU;
	}
	if (SweepPathTexParamV.Num() > 0)
	{
		SweepGen.PathTexCoord = SweepPathTexParamV;
	}*/
	SweepGen.bPolygroupPerQuad = (SweepOptions.PolygroupMode == EGeometryScriptPrimitivePolygroupMode::PerQuad);
	SweepGen.InitialFrame = FFrame3d(SweepGen.Path[0]);
	SweepGen.StartScale = SweepOptions.StartEndRadius.X;
	SweepGen.EndScale = SweepOptions.StartEndRadius.Y;

	

	
	FGeometryScriptPrimitiveOptions PrimitiveOptions;
	PrimitiveOptions.bFlipOrientation = SweepOptions.bFlipOrientation;
	PrimitiveOptions.UVMode = SweepOptions.UVMode;
	PrimitiveOptions.PolygroupMode = SweepOptions.PolygroupMode;
	
	SweepGen.Generate();

	

	GodtierMeshPrimitiveFunctions::AppendPrimitive(TargetMesh, &SweepGen, FTransform::Identity, PrimitiveOptions);
	return TargetMesh;
}

UDynamicMesh* UGodtierModelingUtilities::GenerateCollisionGeometryAlongSpline(FSimpleCollisionOptions CollisionOptions, const ESplineCoordinateSpace::Type Space, UGeometryScriptDebug* Debug)
{
	auto TargetMesh = CollisionOptions.TargetMesh;
	auto Spline = CollisionOptions.Spline;
	
	struct FSplineCache
	{
		TArray<FVector3d> Vertices;
		bool bClosed;
		FTransform ComponentTransform;
	};

	FSplineCache SplineCache;
	
	if (TargetMesh == nullptr)
	{
		UE::Geometry::AppendError(Debug, EGeometryScriptErrorType::InvalidInputs, LOCTEXT("AppendSweepPolyline_NullMesh", "AppendSweepPolyline: TargetMesh is Null"));
		return TargetMesh;
	}
	if (Spline->GetNumberOfSplinePoints() < 2)
	{
		UE::Geometry::AppendError(Debug, EGeometryScriptErrorType::InvalidInputs, LOCTEXT("AppendSweepPolyline_InvalidSweepPath", "AppendSweepPolyline: SweepPath array requires at least 2 positions"));
		return TargetMesh;
	}

	if (CollisionOptions.ZOffset > 0)
	{
		for (int i = 0; i < Spline->GetNumberOfSplinePoints(); ++i)
		{
			const FVector& CurrentLoc = Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
			Spline->SetLocationAtSplinePoint(i, CurrentLoc + FVector(0, 0, CollisionOptions.ZOffset), ESplineCoordinateSpace::Local);
		}
	}

	// GENERATE THE SPLINE CACHE DATA
	Spline->ConvertSplineToPolyLine(Space, CollisionOptions.ErrorTolerance * CollisionOptions.ErrorTolerance, SplineCache.Vertices);
	
	if (SplineCache.Vertices.Num() < 2)
	{
		UE::Geometry::AppendError(Debug, EGeometryScriptErrorType::InvalidInputs, LOCTEXT("AppendSweepPolyline_InvalidPolygon", "AppendSweepPolyline: Polyline array requires at least 2 positions"));
		return TargetMesh;
	}
	
	const TUniquePtr<FTriangulateCurvesOp> NewGeo = MakeUnique<FTriangulateCurvesOp>();
	NewGeo->AddSpline(Spline, CollisionOptions.ErrorTolerance * CollisionOptions.ErrorTolerance);
	NewGeo->Thickness = CollisionOptions.Height;
	NewGeo->UVScaleFactor = 1.0;
	NewGeo->bWorldSpaceUVScale = true;
	NewGeo->FlattenMethod = EFlattenCurveMethod::AlongZ;
	NewGeo->OffsetClosedMethod = EOffsetClosedCurvesMethod::OffsetBothSides;
	NewGeo->OffsetOpenMethod = EOffsetOpenCurvesMethod::Offset;
	NewGeo->CombineMethod = ECombineCurvesMethod::Union;
	NewGeo->OffsetJoinMethod = EOffsetJoinMethod::Square;
	NewGeo->OpenEndShape = EOpenCurveEndShapes::Butt;
	NewGeo->CurveOffset = CollisionOptions.Width;
	
	//NewGeo->AddWorldCurve(SplineCache.Vertices, Spline->IsClosedLoop(), SplineCache.ComponentTransform);


	FProgressCancel Progress;
	
	NewGeo->CalculateResult(&Progress);
	
	auto DynamicGeo = NewGeo->ExtractResult();
	
	TargetMesh->SetMesh(*DynamicGeo.Get());
	
	return TargetMesh;
}

UDynamicMesh* UGodtierModelingUtilities::GenerateMeshFromPlanarFace(ADynamicMeshActor* ParentActor, AActor* TargetActor, const FVector NormalDirection, UGeometryScriptDebug* Debug)
{
	FGeometryScriptMeshSelection Selection;
	FGeometryScriptMeshSelection NewSelection;

	if(!ParentActor || !TargetActor) return nullptr;

	auto TempMesh = ParentActor->AllocateComputeMesh();
	TempMesh->Reset();
	

	if (TargetActor->IsA(ADynamicMeshActor::StaticClass()))
	{
		TempMesh->SetMesh(*Cast<ADynamicMeshActor>(TargetActor)->GetDynamicMeshComponent()->GetMesh());
	}
	else if (TargetActor->IsA(AStaticMeshActor::StaticClass()))
	{
		
		EGeometryScriptOutcomePins Outcome;
		TempMesh->SetMesh(
			*UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromStaticMeshV2
		(
			Cast<AStaticMeshActor>(TargetActor)->GetStaticMeshComponent()->GetStaticMesh(),
			TempMesh,
			FGeometryScriptCopyMeshFromAssetOptions(),
			FGeometryScriptMeshReadLOD(),
			Outcome
		)->ExtractMesh().Get());
	}
	
	UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsByNormalAngle(TempMesh, Selection, NormalDirection);

	UGeometryScriptLibrary_MeshSelectionFunctions::InvertMeshSelection(TempMesh, Selection, NewSelection);

	int32 NumDeleted;
		
	return UGeometryScriptLibrary_MeshBasicEditFunctions::DeleteSelectedTrianglesFromMesh(TempMesh, NewSelection, NumDeleted);
	
	
}

#undef LOCTEXT_NAMESPACE
