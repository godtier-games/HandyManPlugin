// Fill out your copyright notice in the Description page of Project Settings.


#include "ModelingUtilities/GodtierModelingUtilities.h"

#include "DynamicMeshEditor.h"
#include "MatrixTypes.h"
#include "UDynamicMesh.h"
#include "Components/SplineComponent.h"
#include "DynamicMesh/MeshTransforms.h"
#include "Generators/SweepGenerator.h"
#include "GeometryScript/PolyPathFunctions.h"

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

UDynamicMesh* UGodtierModelingUtilities::SweepGeometryAlongSpline(FSweepOptions SweepOptions, ESplineCoordinateSpace::Type Space, UGeometryScriptDebug* Debug)
{
	auto TargetMesh = SweepOptions.TargetMesh;
	auto Spline = SweepOptions.Spline;
	
	TArray<FVector2D> PolylineVertices;
	TArray<FTransform> SweepPath;
	
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
	
	switch (SweepOptions.ShapeType)
	{
		case ESweepShapeType::Circle:
			{
				auto GeometryPath = UGeometryScriptLibrary_PolyPathFunctions::CreateCirclePath3D(FTransform::Identity, SweepOptions.ShapeRadius, SweepOptions.ShapeSegments);
				for (int i = 0; i < GeometryPath.Path->Num(); i++)
				{
					PolylineVertices.Add(FVector2D(GeometryPath.Path->GetData()[i].X, GeometryPath.Path->GetData()[i].Y));
				}
				PolylineVertices.Emplace(*GeometryPath.Path->GetData());
				break;
			}
		case ESweepShapeType::Triangle:
			{
				auto GeometryPath = UGeometryScriptLibrary_PolyPathFunctions::CreateCirclePath3D(FTransform::Identity, SweepOptions.ShapeRadius, 3);
				for (int i = 0; i < GeometryPath.Path->Num(); i++)
				{
					PolylineVertices.Add(FVector2D(GeometryPath.Path->GetData()[i].X, GeometryPath.Path->GetData()[i].Y));
				}
				break;
			}
		case ESweepShapeType::Box:
			{
				PolylineVertices.Add(FVector2D(0.0f, 0.0f));
				PolylineVertices.Add(FVector2D(SweepOptions.ShapeDimensions.X, 0.0f));
				PolylineVertices.Add(SweepOptions.ShapeDimensions);
				PolylineVertices.Add(FVector2D(0.0f, SweepOptions.ShapeDimensions.Y));

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
					PolylineVertices.Add(FVector2D(GeometryPath.Path->GetData()[i].X, GeometryPath.Path->GetData()[i].Y));
				}
				break;
			}
	}
	
	if (PolylineVertices.Num() < 2)
	{
		UE::Geometry::AppendError(Debug, EGeometryScriptErrorType::InvalidInputs, LOCTEXT("AppendSweepPolyline_InvalidPolygon", "AppendSweepPolyline: Polyline array requires at least 2 positions"));
		return TargetMesh;
	}

	for (int i = 0; i < Spline->GetNumberOfSplinePoints(); i++)
	{
		SweepPath.Add(Spline->GetTransformAtSplinePoint(i, Space));
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
	for (const FVector2D& Point : PolylineVertices)
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

#undef LOCTEXT_NAMESPACE
