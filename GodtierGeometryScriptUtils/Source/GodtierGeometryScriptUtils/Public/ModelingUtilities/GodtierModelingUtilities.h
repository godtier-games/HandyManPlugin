// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModelingOperators.h"
#include "ModelingUtilitiesDataTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/SplineComponent.h"
#include "Physics/PhysicsDataCollection.h"
#include "ShapeApproximation/MeshSimpleShapeApproximation.h"
#include "GodtierModelingUtilities.generated.h"

class ADynamicMeshActor;
/**
 * 
 */
UCLASS()
class GODTIERGEOMETRYSCRIPTUTILS_API UGodtierModelingUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (ScriptMethod, DisplayName = "Sweep Geometry", Keywords = "Sweep Geometry Pipe Curve"), Category = "GodtierGeometryScriptUtils | Modeling Utilities")
	static UPARAM(DisplayName = "Output Mesh") UDynamicMesh* SweepGeometryAlongSpline(FSweepOptions SweepOptions, const ESplineCoordinateSpace::Type Space = ESplineCoordinateSpace::World, UGeometryScriptDebug* Debug = nullptr);

	UFUNCTION(BlueprintCallable, meta = (ScriptMethod, DisplayName = "Create Planar Mesh From Spline", Keywords = "Sweep Geometry Pipe Curve"), Category = "GodtierGeometryScriptUtils | Modeling Utilities")
	static UPARAM(DisplayName = "Output Mesh") UDynamicMesh* GenerateCollisionGeometryAlongSpline(FSimpleCollisionOptions CollisionOptions, const ESplineCoordinateSpace::Type Space, UGeometryScriptDebug* Debug = nullptr);

	UFUNCTION(BlueprintCallable, meta = (ScriptMethod, DisplayName = "Extract Planar Mesh From Mesh", Keywords = "Sweep Geometry Pipe Curve"), Category = "GodtierGeometryScriptUtils | Modeling Utilities")
    static UPARAM(DisplayName = "Output Mesh") UDynamicMesh* GenerateMeshFromPlanarFace(UDynamicMesh* ComputeMesh, UDynamicMesh* FromMesh, const FVector NormalDirection = FVector::UpVector, UGeometryScriptDebug* Debug = nullptr);

	/*Take an input mesh, and use it to generate a collision mesh, then convert that collision to a mesh that can be used as a boolean mesh
	 * If the boolean shape is set to "Exact", the input mesh will be used as the base shape for the boolean operation
	 */
	UFUNCTION(BlueprintCallable, meta = (ScriptMethod, DisplayName = "Create Boolean Mesh From Mesh", Keywords = "Boolean Mesh subtract union intersect"), Category = "GodtierGeometryScriptUtils | Modeling Utilities")
	static UPARAM(DisplayName = "Output Mesh") UDynamicMesh* CreateDynamicBooleanMesh(UDynamicMesh* ComputeMesh, AActor* TargetActor, const EMeshBooleanShape BooleanShape, const float IntersectionOffset = 0.5f, UGeometryScriptDebug* Debug = nullptr);
	

	
};
