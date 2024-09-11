// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PCGComponent.h"
#include "Components/SplineComponent.h"
#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_ActorBase.h"
#include "ToolSet/HandyManTools/PCG/SplineTool/Interface/SplineToolInterface.h"
#include "PCG_SplineActor.generated.h"


UCLASS(meta = (PrioritizeCategories = ProceduralSettings))
class HANDYMAN_API APCG_SplineActor : public APCG_ActorBase, public ISplineToolInterface
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

	virtual void SetSplinePoints(const TArray<FTransform> Points) override;
	virtual void SetSplinePoints_Vector(const TArray<FVector> Points) override;

	virtual void SetCloseSpline(bool bCloseLoop) override
	{
		if (SplineComponent)
		{
			SplineComponent->SetClosedLoop(bCloseLoop, true);

			if (PCGComponent)
			{
				PCGComponent->NotifyPropertiesChangedFromBlueprint();
			}
		}
	}

	virtual void SetSplineMesh(const TSoftObjectPtr<UStaticMesh> Mesh) override
	{
		SplineMesh = Mesh;
		SplineMeshPath = SplineMesh.ToSoftObjectPath();
		RerunConstructionScripts();
	};

	virtual void SetEnableRandomRotation(const bool bEnable) override
	{
		bEnableRandomRotation = bEnable;
		RerunConstructionScripts();
	};

	virtual void SetAimMeshAtNextPoint(const bool bEnable) override
	{
		bAimMeshAtNextPoint = bEnable;
		RerunConstructionScripts();
	};

	virtual void SetMinRandomRotation(const FRotator Rotation) override
	{
		MinRandomRotation = Rotation;
		RerunConstructionScripts();
	};

	virtual void SetMaxRandomRotation(const FRotator Rotation) override
	{
		MaxRandomRotation = Rotation;
		RerunConstructionScripts();
	};

	virtual void SetMeshScale(const FVector2D HeightRange) override
	{
		MeshHeightRange = HeightRange;
		RerunConstructionScripts();
	};

	virtual void SetMeshOffsetDistance(const float OffsetDistance) override
	{
		MeshOffsetDistance = OffsetDistance;
		RerunConstructionScripts();
	};

	virtual void SetSplinePointType(const ESplinePointType::Type PointType) override
	{
		SplinePointType = PointType;
		if (SplineComponent)
		{
			for (int i = 0; i < SplineComponent->GetNumberOfSplinePoints(); i++)
			{
				SplineComponent->SetSplinePointType(i, PointType);
			}

			PCGComponent->NotifyPropertiesChangedFromBlueprint();
		}
	};
	
	virtual UPCGComponent* GetPCGComponent() const override { return PCGComponent; }
	

protected:


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProceduralSettings)
	TSoftObjectPtr<UStaticMesh> SplineMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProceduralSettings)
	bool bEnableRandomRotation = false;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProceduralSettings)
	bool bAimMeshAtNextPoint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = "bEnableRandomRotation", EditConditionHides), Category = ProceduralSettings)
	FRotator MinRandomRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = "bEnableRandomRotation", EditConditionHides), Category = ProceduralSettings)
	FRotator MaxRandomRotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProceduralSettings)
	FVector2D MeshHeightRange = FVector2D(1.0f, 1.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProceduralSettings)
	float MeshOffsetDistance = 100.0f;

	UPROPERTY(BlueprintReadOnly)
	FSoftObjectPath SplineMeshPath;
	
	UPROPERTY(BlueprintReadWrite, Category = "HandyMan | Procedural Settings")
	TEnumAsByte<ESplinePointType::Type> SplinePointType = ESplinePointType::Type::Linear;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandyMan")
	TObjectPtr<class USplineComponent> SplineComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandyMan")
	TObjectPtr<class UPCGComponent> PCGComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandyMan")
	TObjectPtr<class USceneComponent> DefaultSceneComponent;



private:
	
};
