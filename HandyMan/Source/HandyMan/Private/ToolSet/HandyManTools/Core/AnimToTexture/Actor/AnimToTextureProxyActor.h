// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManTools/PCG/Core/Actors/PCG_DynamicMeshActor_Editor.h"
#include "AnimToTextureProxyActor.generated.h"

UCLASS()
class HANDYMAN_API AAnimToTextureProxyActor : public APCG_DynamicMeshActor_Editor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAnimToTextureProxyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
