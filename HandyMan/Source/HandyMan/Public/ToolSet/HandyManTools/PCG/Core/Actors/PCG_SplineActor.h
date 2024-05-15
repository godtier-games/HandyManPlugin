// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGComponent.h"
#include "PCG_ActorBase.h"
#include "HoudiniEngineEditor/Private/HoudiniEngineToolTypes.h"
#include "PCG_SplineActor.generated.h"

UCLASS(meta = (PrioritizeCategories = ProceduralSettings))
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
	void SetCloseSpline(bool bCloseLoop)
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
	void SetAimMeshAtNextPoint(const bool bEnable)
	{
		bAimMeshAtNextPoint = bEnable;
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
	
	UFUNCTION(BlueprintCallable, Category = "HandyMan")
	void SetSplinePointType(const ESplinePointType::Type& PointType)
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

	UFUNCTION(BlueprintPure, Category = "HandyMan")
	UPCGComponent* GetPCGComponent() const { return PCGComponent; }
	

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
	TEnumAsByte<ESplinePointType::Type> SplinePointType = ESplinePointType::Linear;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandyMan")
	TObjectPtr<class USplineComponent> SplineComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandyMan")
	TObjectPtr<class UPCGComponent> PCGComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandyMan")
	TObjectPtr<class USceneComponent> DefaultSceneComponent;



private:
	
};
