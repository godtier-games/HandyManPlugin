#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "HandyManBuildingTypes.generated.h"


USTRUCT(BlueprintType)
struct FHandyManBuildingMeshComponent
{
	GENERATED_BODY()

	

	FHandyManBuildingMeshComponent(): Mesh(nullptr)
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Mesh Component")
	FGameplayTag MeshNameTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Mesh Component")
	UStaticMesh* Mesh;
	
};


USTRUCT(BlueprintType)
struct FHandyManFloorLayout
{
	GENERATED_BODY()

	

	FHandyManFloorLayout()
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Building Component")
	FGameplayTag FloorNameTag;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Building Component")
	float FloorHeight = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Building Component")
	TArray<FHandyManBuildingMeshComponent> Pattern;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Building Component")
	bool bChooseSpecificCornerMesh = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition = "bChooseSpecificCornerMesh", EditConditionHides), Category = "Handy Man Building Component")
	FHandyManBuildingMeshComponent CornerMesh;
	
	
};

/*Converts to logic to pass into the build pattern input on the HDA*/
USTRUCT(BlueprintType)
struct FHandyManBuildingLayout
{
	GENERATED_BODY()

	

	FHandyManBuildingLayout()
	{
	}
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Building Component")
	AActor* TargetBlockoutMesh = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Building Component")
	TArray<FHandyManFloorLayout> FloorLayout;

	
};


/*Special type that will be used in the Data Table that is sent to houdini that has data about the meshes*/
USTRUCT(BlueprintType)
struct FHandyManHoudiniFloorModule
{
	GENERATED_BODY()

	

	FHandyManHoudiniFloorModule(): Mesh(nullptr)
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Mesh Component")
	FString MeshName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Mesh Component")
	UStaticMesh* Mesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Mesh Component")
	float MeshWidth = 0.0f;
	
};

/*Special type that will be used in the Data Table that is sent to houdini that has data about the floor plan*/
USTRUCT(BlueprintType)
struct FHandyManHoudiniBuildingModule
{
	GENERATED_BODY()

	

	FHandyManHoudiniBuildingModule()
	{
	}
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Building Component")
	AActor* TargetBlockoutMesh = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Handy Man Building Component")
	TArray<FHandyManFloorLayout> FloorLayout;
	
};


UENUM()
enum class EBuildingSectionType
{
	None,
	Wall,
	Window
};
