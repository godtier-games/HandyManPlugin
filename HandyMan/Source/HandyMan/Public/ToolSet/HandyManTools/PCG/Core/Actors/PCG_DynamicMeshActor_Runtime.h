// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "PCG_DynamicMeshActor_Runtime.generated.h"

class UPCGComponent;

UCLASS()
class HANDYMAN_API APCG_DynamicMeshActor_Runtime : public ADynamicMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APCG_DynamicMeshActor_Runtime();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "PCG")
	virtual UPCGComponent* GetPCGComponent() const { return nullptr;}

	UFUNCTION(BlueprintCallable, Category = "PCG")
	void SetCollisionProfileByName(const FName ProfileName);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dynamic Mesh", meta=(GetOptions="HandyMan.HandyManStatics.GetCollisionProfileNames"))
	FName CollisionProfileName = FName(TEXT("BlockAll"));
	
	
	/** Handle for OnMeshObjectChanged which is registered with MeshObject::OnMeshChanged delegate */
	FDelegateHandle MeshObjectChangedHandle;

	virtual void OnMeshObjectChanged(UDynamicMesh* ChangedMeshObject, FDynamicMeshChangeInfo ChangeInfo);
};
