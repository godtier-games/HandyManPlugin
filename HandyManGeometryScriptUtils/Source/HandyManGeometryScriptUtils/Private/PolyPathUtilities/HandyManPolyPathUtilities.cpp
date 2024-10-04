// Fill out your copyright notice in the Description page of Project Settings.


#include "PolyPathUtilities/HandyManPolyPathUtilities.h"

#include "GeometryScript/GeometryScriptSelectionTypes.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "GeometryScript/MeshSelectionQueryFunctions.h"


TArray<FGeometryScriptPolyPath> UHandyManPolyPathUtilities::CreatePolyPathFromPlanarFaces(UDynamicMesh* TargetMesh, const TArray<FVector> NormalDirections, UGeometryScriptDebug* Debug)
{
	TArray<FGeometryScriptPolyPath> OutPaths;
	for (const auto Direction : NormalDirections)
	{
		FGeometryScriptMeshSelection Selection;

		TArray<FGeometryScriptPolyPath> FoundPaths;
		TArray<FGeometryScriptIndexList> FoundIndices;
		int32 numLoops;
		bool bFoundErrors;
		UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsByNormalAngle(TargetMesh, Selection, Direction);
		UGeometryScriptLibrary_MeshSelectionQueryFunctions::GetMeshSelectionBoundaryLoops(TargetMesh, Selection, FoundIndices, FoundPaths, numLoops, bFoundErrors);

		OutPaths.Append(FoundPaths);
	}
	
	return OutPaths;
	
}
