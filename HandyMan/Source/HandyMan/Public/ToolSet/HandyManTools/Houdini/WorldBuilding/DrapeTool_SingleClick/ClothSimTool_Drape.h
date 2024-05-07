// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/DataTypes/HandyManDataTypes.h"
#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"
#include "ClothSimTool_Drape.generated.h"

UENUM()
enum class EDrapeToolStage_SingleClick
{
	Beginning,
	Collision,
	Drape,
	Pin,
	Finish,
	EditSelection,
	EditGeometry,
	Simulate,
};

UCLASS()
class HANDYMAN_API UClothSimToolActions : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	TWeakObjectPtr<UClothSimTool_Drape> ParentTool;

	void Initialize(UClothSimTool_Drape* ParentToolIn) { ParentTool = ParentToolIn; }

	void PostAction(EDrapeToolStage_SingleClick Action);

	/*UFUNCTION(CallInEditor, Category = Actions, meta = (DisplayPriority = 1))
	void SetClothGeo() { PostAction(EDrapeToolStage_SingleClick::Drape); }

	UFUNCTION(CallInEditor, Category = Actions, meta = (DisplayPriority = 2))
	void SetPinGeo() { PostAction(EDrapeToolStage_SingleClick::Pin); }

	UFUNCTION(CallInEditor, Category = Actions, meta = (DisplayPriority = 3))
	void SetCollision() { PostAction(EDrapeToolStage_SingleClick::Collision); }

	UFUNCTION(CallInEditor, Category = Edit, meta = (DisplayPriority = 1))
	void EditSelections() { PostAction(EDrapeToolStage_SingleClick::EditSelection); }

	UFUNCTION(CallInEditor, Category = Edit, meta = (DisplayPriority = 2))
	void EditGeometry() { PostAction(EDrapeToolStage_SingleClick::EditGeometry); }
	*/

	UFUNCTION(CallInEditor, Category = Simulation, meta = (DisplayPriority = 1))
	void SimulateCloth() { PostAction(EDrapeToolStage_SingleClick::Simulate); }
	
};


/**
 * 
 */
UCLASS()
class HANDYMAN_API UClothSimTool_Drape : public UHandyManSingleClickTool
{
	GENERATED_BODY()

public:

	UClothSimTool_Drape();
	
	void RequestAction(const EDrapeToolStage_SingleClick Action);
	void StartWatchingProperties();


	virtual void Setup() override;

	// Click
	//virtual FInputRayHit TestIfHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;
	virtual void OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;
	
	virtual void OnTick(float DeltaTime) override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	virtual bool HasCancel() const override {return true;}
	virtual bool HasAccept() const override {return true;}
	virtual bool CanAccept() const override {return bCanAcceptTool;}


private:

	bool bCanAcceptTool = false;
	UPROPERTY()
	class UClothSimPropertySettings* Settings;

	UPROPERTY()
	class UClothSimToolActions* Actions;

	UPROPERTY()
	class UHoudiniPublicAPIAssetWrapper* CurrentInstance;

	UPROPERTY()
	TMap<EDrapeToolStage_SingleClick, FObjectSelection> SelectedActors;
};







UCLASS()
class HANDYMAN_API UClothSimPropertySettings : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta = (Tooltip = "Toggle the option to keep the original geometry for further editing"))
	bool bKeepInputClothGeometry = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta = (Tooltip = "Toggle the option to hide the original geometry. This tool duplicates it and spits out a new one."))
	bool bHideInputClothGeometry = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Geometry", meta = (Tooltip = "Select the plane or polygon that the tool will run on. Position this geometry in the world where you want the cloth to drape. It should be above the collision geo if any."))
	AStaticMeshActor* ClothGeometry;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Geometry", meta = (Tooltip = "Select an array of actors that the tool will use as colliders when simulating the cloth. This geometry should not overlap the cloth geometry."))
	TArray<AStaticMeshActor*> CollisionGeometry;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Geometry", meta = (Tooltip = "Select an array of actors that the tool will use as pins in the simulation. This geometry should overlap the cloth geometry."))
	TArray<AStaticMeshActor*> PinGeometry;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta = (Tooltip = "Increase the amount of polygon for a better simulation at the cost of speed.",
		UIMin = "100", UIMax = "25000", ClampMin = "100", ClampMax = "50000"))
	int32 MaxPolyCount = 1000;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta = (Tooltip = "This will affect how close your mesh gets to its target polygon count. A higher value will result in a more accurate mesh but will take longer to compute.",
		UIMin = "1", UIMax = "5", ClampMin = "1", ClampMax = "5"))
	int32 RemeshPrecision = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta = (Tooltip = "This setting will smooth out the topology flow of the simulation mesh. A higher value will result in a smoother mesh with more equal polygons.",
		UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
	float RemeshSmoothing = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta = (Tooltip = "This will affect your meshes normals",
		UIMin = "0", UIMax = "180", ClampMin = "0", ClampMax = "180"))
	int32 NormalAngle = 60;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta = (Tooltip = "This setting will change the resolution of the UVs on the final geometry.",
		UIMin = "0.1", UIMax = "3", ClampMin = "0.1", ClampMax = "3"))
	float UvScale = 25;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta = (Tooltip = "Toggle the option to run geometry reduction on the final sim. Best to do this only when you want to bake out the final geometry"))
	bool bOverrideMaterial = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta=(EditCondition="bOverrideMaterial", EditConditionHides, Tooltip = "Select the material that will be applied to the final geometry"))
	UMaterialInterface* ClothMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta = (Tooltip = "Toggle the option to run geometry reduction on the final sim. Best to do this only when you want to bake out the final geometry"))
	bool bOptimizeGeometry = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta = (Tooltip = "This setting will change the final geometry's polygon count",
		UIMin = "1", UIMax = "100", ClampMin = "1", ClampMax = "100", EditCondition="bOptimizeGeometry", EditConditionHides))
	float PercentageOfGeometryToKeep = 25;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta = (Tooltip = "This setting will change the final geometry's polygon count",
		UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
	float PostSimulationSmoothAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Geometry Settings", meta = (Tooltip = "This setting will change the final geometry's polygon count",
		UIMin = "0", UIMax = "3", ClampMin = "0", ClampMax = "3"))
	float PostSimulationThickness = 0.1f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Simulation Settings", meta = (Tooltip = "This will output the selected frame as the final product. Add or subtract from this value to get the desired simulation frame."))
	int32 OutputFrame = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Simulation Settings", meta = (Tooltip = "This will increase the time step of the simulation. Use this as a cheap way to get more frames at the cost of lower sim quality (E.G - 1.5 Time Scale & Output Frame 30 = 45 frames of simulation)",
		UIMin = "1", UIMax = "100", ClampMin = "1", ClampMax = "100"))
	float TimeScale = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Simulation Settings", meta = (Tooltip = "How stretchy the cloth is. A higher value will result in a longer cloth with slight UV Stretching.",
		UIMin = "0.2", UIMax = "100", ClampMin = "0.2", ClampMax = "100"))
	float ClothStretch = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Simulation Settings", meta = (Tooltip = "How will this cloth slide against itself & other services. A higher value will result in less sliding.",
		UIMin = "0.2", UIMax = "100", ClampMin = "0.2", ClampMax = "100"))
	float ClothStiffness = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Simulation Settings", meta = (Tooltip = "How will this cloth slide against itself & other services. A higher value will result in less sliding.",
		UIMin = "0.2", UIMax = "100", ClampMin = "0.2", ClampMax = "100"))
	float ClothFriction = 1;
};




