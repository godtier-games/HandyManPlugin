// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModelingUtilitiesDataTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/SplineComponent.h"
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
	static UPARAM(DisplayName = "Output Mesh") UDynamicMesh* SweepGeometryAlongSpline(FSweepOptions SweepOptions, const ESplineCoordinateSpace::Type Space, UGeometryScriptDebug* Debug = nullptr);

	UFUNCTION(BlueprintCallable, meta = (ScriptMethod, DisplayName = "Create Planar Mesh From Spline", Keywords = "Sweep Geometry Pipe Curve"), Category = "GodtierGeometryScriptUtils | Modeling Utilities")
	static UPARAM(DisplayName = "Output Mesh") UDynamicMesh* GenerateCollisionGeometryAlongSpline(FSimpleCollisionOptions CollisionOptions, const ESplineCoordinateSpace::Type Space, UGeometryScriptDebug* Debug = nullptr);

	UFUNCTION(BlueprintCallable, meta = (ScriptMethod, DisplayName = "Extract Planar Mesh From Mesh", Keywords = "Sweep Geometry Pipe Curve"), Category = "GodtierGeometryScriptUtils | Modeling Utilities")
    static UPARAM(DisplayName = "Output Mesh") UDynamicMesh* GenerateMeshFromPlanarFace(ADynamicMeshActor* ParentActor, AActor* TargetActor, const FVector NormalDirection = FVector::UpVector, UGeometryScriptDebug* Debug = nullptr);
};
