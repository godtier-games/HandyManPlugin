// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCG_ActorBase.h"
#include "PCG_SplineActor.generated.h"

UCLASS()
class HANDYMAN_API APCG_SplineActor : public APCG_ActorBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APCG_SplineActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	void SetSplinePoints(const TArray<FTransform>& Points);

	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	void SetSplineMesh(const TSoftObjectPtr<UStaticMesh> Mesh)
	{
		SplineMesh = Mesh;
		SplineMeshPath = SplineMesh.ToSoftObjectPath();
		RerunConstructionScripts();
	};
	
	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	void SetEnableRandomRotation(const bool bEnable)
	{
		bEnableRandomRotation = bEnable;
		RerunConstructionScripts();
	};

	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	void SetMinRandomRotation(const FRotator Rotation)
	{
		MinRandomRotation = Rotation;
		RerunConstructionScripts();
	};

	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	void SetMaxRandomRotation(const FRotator Rotation)
	{
		MaxRandomRotation = Rotation;
		RerunConstructionScripts();
	};

	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	void SetMeshHeightRange(const FVector2D HeightRange) 
	{
		MeshHeightRange = HeightRange;
		RerunConstructionScripts();
	};
	
	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	void SetMeshOffsetDistance(const float OffsetDistance) 
	{
		MeshOffsetDistance = OffsetDistance;
		RerunConstructionScripts();
	};
	

protected:



	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandyMan")
	TObjectPtr<class USplineComponent> SplineComponent;

	UPROPERTY(BlueprintReadOnly, Category = "HandyMan")
	TSoftObjectPtr<UStaticMesh> SplineMesh;
	
	UPROPERTY(BlueprintReadOnly, Category = "HandyMan")
	bool bEnableRandomRotation = false;

	UPROPERTY(BlueprintReadOnly, Category = "HandyMan")
	FRotator MinRandomRotation;

	UPROPERTY(BlueprintReadOnly, Category = "HandyMan")
	FRotator MaxRandomRotation;
	
	UPROPERTY(BlueprintReadOnly, Category = "HandyMan")
	FVector2D MeshHeightRange = FVector2D(1.0f, 1.0f);
	
	UPROPERTY(BlueprintReadOnly, Category = "HandyMan")
	float MeshOffsetDistance = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandyMan")
	FSoftObjectPath SplineMeshPath;
};
