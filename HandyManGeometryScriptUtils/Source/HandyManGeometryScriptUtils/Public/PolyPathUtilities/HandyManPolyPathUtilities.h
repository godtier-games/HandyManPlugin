// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryScript/GeometryScriptTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HandyManPolyPathUtilities.generated.h"

class UDynamicMesh;
/**
 * 
 */
UCLASS()
class HANDYMANGEOMETRYSCRIPTUTILS_API UHandyManPolyPathUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (ScriptMethod, DisplayName = "Create Poly Line From Planar Faces", Keywords = "Sweep Geometry Pipe Curve"), Category = "HandyManGeometryScriptUtils | Modeling Utilities")
	static UPARAM(DisplayName = "Paths") TArray<FGeometryScriptPolyPath> CreatePolyPathFromPlanarFaces(UDynamicMesh* TargetMesh, const TArray<FVector> NormalDirections, UGeometryScriptDebug* Debug = nullptr);

};
