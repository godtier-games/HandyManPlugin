// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGData.h"
#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_ActorBase.h"
#include "PCG_IvyActor.generated.h"

USTRUCT()
struct FTempSplinePoint
{
	GENERATED_BODY()

	UPROPERTY()
	FVector StartLocation;

	UPROPERTY()
	FVector StartTangent;

	UPROPERTY()
	FVector EndLocation;

	UPROPERTY()
	FVector EndTangent;

	
};

class UPCGComponent;

UCLASS()
class HANDYMAN_API APCG_IvyActor : public APCG_ActorBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APCG_IvyActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetDisplayMesh(TSoftObjectPtr<UStaticMesh> Mesh) {MeshToGiveVines = Mesh;};

	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetDisplayMeshTransform(const FTransform& NewTransform) {DisplayMesh->SetWorldTransform(NewTransform);};
	
	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetVineThickness(const float Thickness);
	


	UFUNCTION(meta=(CallInEditor="true"))
	void GenerateVines(const FPCGDataCollection& Data);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:

	virtual UPCGComponent* GetPCGComponent() const override {return PCG;}

protected:

	FDelegateHandle DelegateHandle;

	UPROPERTY(BlueprintReadWrite, Category="Inputs")
	bool bRefreshDelegates = false;

	UPROPERTY(BlueprintReadWrite, Category="Inputs")
	TArray<AActor*> TargetActors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPCGComponent> PCG;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> DisplayMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inputs")
	TSoftObjectPtr<UStaticMesh> MeshToGiveVines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inputs")
	TSoftObjectPtr<UStaticMesh> VineMesh;

private:

	UPROPERTY()
	TArray<class USplineMeshComponent*> Vines;
};
