#pragma once
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"

#include "HandyManDataTypes.generated.h"

UENUM()
enum class EDrawSplineDrawMode_HandyMan : uint8
{
	// Click to place a point and then drag to set its tangent. Clicking without
	// dragging will create sharp corners.
	TangentDrag,
	// Click and drag new points, with the tangent set automatically
	ClickAutoTangent,
	// Drag to place multiple points, with spacing controlled by Min Point Spacing
	FreeDraw,

	None,
};

UENUM()
enum class ESplineOffsetMethod_HandyMan : uint8
{
	// Spline points will be offset along the normal direction of the clicked surface
	HitNormal,
	// Spline points will be offset along a manually-chosen direction
	Custom
};

UENUM()
enum class EDrawSplineOutputMode_HandyMan : uint8
{
	// Create a new empty actor with the spline inside it 
	EmptyActor,
	// Attach the spline to an existing actor, or replace a spline inside that
	// actor if Existing Spline Index To Replace is valid.
	ExistingActor,
	// Create the blueprint specified by Blueprint To Create, and either attach
	// the spline to that, or replace an existing spline if Existing Spline Index 
	// To Replace is valid.
	CreateBlueprint,

	//~ The original implementation of this option was too unwieldy, but there is some way to
	//~ implement it using FComponentReference to make a nice spline component picker...
	//SelectedComponent,
};

UENUM()
enum class EDrawSplineUpVectorMode_HandyMan : uint8
{
	// Pick the first up vector based on the hit normal, and then align subsequent
	// up vectors with the previous ones.
	AlignToPrevious,
	// Base the up vector off the hit normal.
	UseHitNormal,
};


USTRUCT(BlueprintType)
struct FObjectSelections
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<UObject*> Selected;

	FObjectSelections()
	{
		Selected.Empty();
	}

	FObjectSelections(UObject* InObject)
	{
		Selected.Empty();
		Selected.Add(InObject);
	};

	FObjectSelections(const TArray<UObject*>& InObjects)
	{
		Selected.Empty();
		Selected = InObjects;
	};

	FObjectSelections(const TArray<AActor*>& InObjects)
	{
		Selected.Empty();
		for (auto Item : InObjects)
		{
			Selected.Add(Cast<UObject>(Item));
		}
	};

	FObjectSelections(const TArray<AStaticMeshActor*>& InObjects)
	{
		Selected.Empty();
		for (auto Item : InObjects)
		{
			Selected.Add(Cast<UObject>(Item));
		}
	};

	
	
};

USTRUCT(BlueprintType)
struct FObjectSelection
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* Selected;

	FObjectSelection()
	{
		Selected = nullptr;
	}

	FObjectSelection(UObject* InObject)
	{
		Selected = Cast<AActor>(InObject);
	};

	FObjectSelection(AActor* InObject)
	{
		Selected = InObject;
	};

	
	
};

USTRUCT(BlueprintType)
struct FHandyManDynamicMeshData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dynamic Mesh Data")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dynamic Mesh Data")
	float Weight = 1.0f;
	
	
};

