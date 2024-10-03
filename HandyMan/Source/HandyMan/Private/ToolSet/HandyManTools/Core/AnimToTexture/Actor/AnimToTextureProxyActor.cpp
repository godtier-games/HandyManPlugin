// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimToTextureProxyActor.h"

#include "GeometryScript/CreateNewAssetUtilityFunctions.h"
#include "GeometryScript/MeshAssetFunctions.h"



// Sets default values
AAnimToTextureProxyActor::AAnimToTextureProxyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAnimToTextureProxyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAnimToTextureProxyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UStaticMesh* AAnimToTextureProxyActor::MakeStaticMesh(USkeletalMesh* SkeletalMesh, const FString& PackageName)
{
	UDynamicMesh* Copy = AllocateComputeMesh();

	FGeometryScriptCopyMeshFromAssetOptions CopyOptions;
	CopyOptions.bApplyBuildSettings = false;

	FGeometryScriptMeshReadLOD CopyLod;
	EGeometryScriptOutcomePins Outcome;
	
	UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromSkeletalMesh(SkeletalMesh, Copy, CopyOptions, CopyLod, Outcome);

	FGeometryScriptCreateNewStaticMeshAssetOptions CreateNewAssetOptions;
	CreateNewAssetOptions.bUseOriginalVertexOrder = true;

	auto GeneratedAsset = UGeometryScriptLibrary_CreateNewAssetFunctions::CreateNewStaticMeshAssetFromMesh(Copy, PackageName, CreateNewAssetOptions, Outcome);;
	ReleaseAllComputeMeshes();
	return GeneratedAsset;
}

