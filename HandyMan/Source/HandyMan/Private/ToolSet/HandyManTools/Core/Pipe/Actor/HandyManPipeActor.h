// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModelingUtilities/ModelingUtilitiesDataTypes.h"
#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_DynamicMeshActor_Editor.h"
#include "HandyManPipeActor.generated.h"

UCLASS()
class HANDYMAN_API AHandyManPipeActor : public APCG_DynamicMeshActor_Editor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHandyManPipeActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	virtual void RebuildGeneratedMesh(UDynamicMesh* TargetMesh) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	TObjectPtr<UMaterialInterface> PipeMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	int32 SampleSize = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	bool bProjectPointsToSurface = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	FVector2D StartEndRadius = FVector2D(1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	int32 ShapeSegments = 16;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	float ShapeRadius = 4.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe Options")
	float RotationAngleDeg = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options, meta = (DisplayName = "PolyGroup Mode"))
	EGeometryScriptPrimitivePolygroupMode PolygroupMode = EGeometryScriptPrimitivePolygroupMode::PerFace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	bool bFlipOrientation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	EGeometryScriptPrimitiveUVMode UVMode = EGeometryScriptPrimitiveUVMode::Uniform;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandyMan")
	TObjectPtr<class USplineComponent> SplineComponent;
};


