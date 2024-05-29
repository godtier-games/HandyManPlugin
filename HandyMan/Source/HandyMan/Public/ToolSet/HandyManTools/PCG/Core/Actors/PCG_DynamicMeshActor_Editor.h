// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryScriptingEditor/Public/GeometryActors/GeneratedDynamicMeshActor.h"
#include "PCG_DynamicMeshActor_Editor.generated.h"

class UPCGComponent;

UCLASS()
class HANDYMAN_API APCG_DynamicMeshActor_Editor : public AGeneratedDynamicMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APCG_DynamicMeshActor_Editor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "PCG")
	virtual UPCGComponent* GetPCGComponent() const { return nullptr; }
};
