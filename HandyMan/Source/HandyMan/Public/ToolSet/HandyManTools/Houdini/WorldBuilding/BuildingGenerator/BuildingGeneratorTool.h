// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "ToolSet/Core/HandyManToolBuilder.h"
#include "ToolSet/DataTypes/HandyManBuildingTypes.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "BuildingGeneratorTool.generated.h"


UCLASS()
class HANDYMAN_API UBuildingGeneratorToolBuilder : public UHandyManToolBuilder
{
	GENERATED_BODY()

public:
	//virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};


UCLASS()
class HANDYMAN_API UBuildingGeneratorProperties : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Building Generator Properties", meta=(ToolTip = "This tool can manage multiple building or just one."))
	TArray<FHandyManBuildingModule> BuildingModules;


	TWeakObjectPtr<UBuildingGeneratorTool> ParentTool;

	void Initialize(UBuildingGeneratorTool* ParentToolIn) { ParentTool = ParentToolIn; }

	void PostAction();

	UFUNCTION(CallInEditor, Category = Generation, meta = (DisplayPriority = 1, ToolTip = "Refresh the building pattern. This should be triggered after any changes to the building modules."))
	void Refresh() { PostAction(); }
	
};



/**
 * 
 */
UCLASS()
class HANDYMAN_API UBuildingGeneratorTool : public UHandyManSingleClickTool
{
	GENERATED_BODY()

public:
	virtual UBaseScriptableToolBuilder* GetNewCustomToolBuilderInstance(UObject* Outer) override;

	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	void RefreshAction();

	
protected:
	UPROPERTY()
	TObjectPtr<UBuildingGeneratorProperties> BuildingModules = nullptr;

	UPROPERTY()
	class UHoudiniPublicAPIAssetWrapper* CurrentInstance;

	

private:

	FString ConstructFloorPatternString(const TArray<FHandyManFloorModule>& Pattern);
	void AppendMeshes(const TArray<FHandyManFloorModule>& Modules, TArray<FHandyManBuildingMeshComponent>& OutMeshes);
	void ConstructMeshDataTable(const TArray<FHandyManBuildingMeshComponent>& Meshes, UDataTable*& OutDataTable);
	void ConstructFloorPatternsDataTable(const TArray<FHandyManFloorModule>& Modules, UDataTable*& OutDataTable);
};
