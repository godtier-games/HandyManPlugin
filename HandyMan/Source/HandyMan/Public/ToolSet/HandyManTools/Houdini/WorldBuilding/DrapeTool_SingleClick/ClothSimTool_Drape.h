// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolSet/HandyManBaseClasses/HandyManClickDragTool.h"
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



/**
 * 
 */
UCLASS()
class HANDYMAN_API UClothSimTool_Drape : public UHandyManClickDragTool
{
	GENERATED_BODY()

public:

	virtual void OnClickPress(const FInputDeviceRay& PressPos) override;

	virtual void OnClickDrag(const FInputDeviceRay& DragPos) override;

	virtual void OnClickRelease(const FInputDeviceRay& ReleasePos) override;
	
	void RequestAction(const EDrapeToolStage_SingleClick Action);
	
	
};




UCLASS()
class HANDYMAN_API UClothSimToolActions : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	TWeakObjectPtr<UClothSimTool_Drape> ParentTool;

	void Initialize(UClothSimTool_Drape* ParentToolIn) { ParentTool = ParentToolIn; }

	void PostAction(EDrapeToolStage_SingleClick Action) {};

	UFUNCTION(CallInEditor, Category = Actions, meta = (DisplayPriority = 1))
	void SetClothGeo() { PostAction(EDrapeToolStage_SingleClick::Drape); }

	UFUNCTION(CallInEditor, Category = Actions, meta = (DisplayPriority = 2))
	void SetPinGeo() { PostAction(EDrapeToolStage_SingleClick::Pin); }

	UFUNCTION(CallInEditor, Category = Actions, meta = (DisplayPriority = 3))
	void SetCollision() { PostAction(EDrapeToolStage_SingleClick::Collision); }

	UFUNCTION(CallInEditor, Category = Edit, meta = (DisplayPriority = 1))
	void EditSelections() { PostAction(EDrapeToolStage_SingleClick::EditSelection); }

	UFUNCTION(CallInEditor, Category = Edit, meta = (DisplayPriority = 2))
	void EditGeometry() { PostAction(EDrapeToolStage_SingleClick::EditGeometry); }

	UFUNCTION(CallInEditor, Category = Simulation, meta = (DisplayPriority = 1))
	void ResetSimulation() { PostAction(EDrapeToolStage_SingleClick::Simulate); }
	
};
