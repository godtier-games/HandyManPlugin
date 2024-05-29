// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_ActorBase.h"
#include "PCG_ScatterMeshActor.generated.h"

UCLASS()
class HANDYMAN_API APCG_ScatterMeshActor : public APCG_ActorBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APCG_ScatterMeshActor();
	
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetDisplayMesh(TSoftObjectPtr<UStaticMesh> Mesh) {MeshToScatterOn = Mesh;}

	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void SetDisplayMeshTransform(const FTransform& NewTransform) {DisplayMesh->SetWorldTransform(NewTransform);};

	UFUNCTION(BlueprintCallable, Category="Handy Man")
	void TransferMeshMaterials(TArray<UMaterialInterface*> Materials);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:

	virtual UPCGComponent* GetPCGComponent() const override {return PCG;}

protected:

	UPROPERTY(BlueprintReadWrite, Category="Inputs")
	TArray<AActor*> TargetActors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPCGComponent> PCG;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> DisplayMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inputs")
	TSoftObjectPtr<UStaticMesh> MeshToScatterOn;


};
