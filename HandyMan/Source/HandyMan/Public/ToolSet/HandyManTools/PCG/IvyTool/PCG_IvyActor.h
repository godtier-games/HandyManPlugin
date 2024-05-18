﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGData.h"
#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_ActorBase.h"
#include "PCG_IvyActor.generated.h"

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

#if WITH_EDITOR
	UFUNCTION(meta=(CallInEditor="true"))
	void GenerateVines(const FPCGDataCollection& Data);
#endif
	

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
