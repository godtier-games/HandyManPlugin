// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SplineToolInterface.generated.h"

namespace ESplinePointType
{
	enum Type : int;
}

// This class does not need to be modified.
UINTERFACE(NotBlueprintable)
class USplineToolInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HANDYMAN_API ISplineToolInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, Category = "HandyMan", meta = (DisplayName = "SetSplinePointsFromTransform"))
	virtual void SetSplinePoints(const TArray<FTransform> Points) = 0;

	UFUNCTION(BlueprintCallable, Category = "HandyMan", meta = (DisplayName = "SetSplinePointsFromVector"))
	virtual void SetSplinePoints_Vector(const TArray<FVector> Points) = 0;

	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	virtual void SetCloseSpline(bool bCloseLoop) = 0;
	
	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	virtual void SetSplineMesh(const TSoftObjectPtr<UStaticMesh> Mesh) = 0;
	
	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	virtual void SetEnableRandomRotation(const bool bEnable) = 0;

	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	virtual void SetAimMeshAtNextPoint(const bool bEnable) = 0;

	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	virtual void SetMeshScale(const FVector2D InMeshScale) = 0;
	
	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	virtual void SetMinRandomRotation(const FRotator Rotation) = 0;

	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	virtual void SetMaxRandomRotation(const FRotator Rotation) = 0;
	
	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	virtual void SetMeshOffsetDistance(const float OffsetDistance) = 0; 
	
	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	virtual void SetSplinePointType(const ESplinePointType::Type PointType) = 0;

	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	virtual void SetColliderZOffset(const float InZOffest) {};
	
	
	
};
