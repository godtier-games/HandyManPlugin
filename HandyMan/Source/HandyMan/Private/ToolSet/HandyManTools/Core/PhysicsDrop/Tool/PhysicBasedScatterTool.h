// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTools/ScriptableModularBehaviorTool.h"
#include "ToolSet/HandyManBaseClasses/HandyManClickDragTool.h"
#include "PhysicBasedScatterTool.generated.h"


/** Reference Mesh Struct */
USTRUCT(BlueprintType)
struct HANDYMAN_API FReferenceMeshData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bShouldPlace = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta=(EditCondition="bShouldPlace", EditConditionHides, ClampMin="0", ClampMax="100"))
	float Chance=100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FVector MinRotate = FVector::Zero();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FVector MaxRotate = FVector::Zero();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FVector MinScale = FVector::One();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FVector MaxScale = FVector::One();
};

/** Layout mode names Enum */
namespace ELayoutMode
{
	static const FString Select = TEXT("Select");
	static const FString PaintSelect = TEXT("Paint Select");
	static const FString Transform = TEXT("Transform");
	static const FString Paint = TEXT("Paint");
}

/** Layout mode colors Enum */
namespace ELayoutModeColor
{
	static const FLinearColor Add = FLinearColor::Green;
	static const FLinearColor Remove = FLinearColor::Red;
	static const FLinearColor Select = FLinearColor::Blue;
	static const FLinearColor Deselect = FLinearColor(1.0f, 1.0f, 0.0f);
}



/**
 * 
 */
UCLASS(Blueprintable)
class HANDYMAN_API UPhysicBasedScatterTool : public UHandyManClickDragTool
{
	GENERATED_BODY()

public:
	UPhysicBasedScatterTool();
	
	void CreateBrush();

	virtual void Setup() override;

	// IHoverBehaviorTarget API
	virtual bool OnUpdateHover(const FInputDeviceRay& DevicePos) override;

	virtual void OnClickDrag(const FInputDeviceRay& DragPos) override;

	bool bCanSpawn = false;

	FVector LatestPosition = FVector::Zero();
	
	//FInputRayHit Test(FInputDeviceRay ClickPos, EScriptableToolMouseButton MouseButton);

	UFUNCTION(BlueprintCallable)
	void OnPressedFunc(FInputDeviceRay ClickPos, FScriptableToolModifierStates Modifiers, EScriptableToolMouseButton MouseButton);

	UFUNCTION(BlueprintCallable)
	void OnReleaseFunc(FInputDeviceRay ClickPos, FScriptableToolModifierStates Modifiers, EScriptableToolMouseButton MouseButton);

	UFUNCTION(BlueprintCallable)
	bool MouseBehaviorModiferCheckFunc(const FInputDeviceState& InputDeviceState);
	
	

	UFUNCTION()
	void ActorSelectionChangeNotify();

	virtual void OnTick(float DeltaTime) override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	virtual void UpdateAcceptWarnings(EAcceptWarning Warning) override;
	virtual bool CanAccept() const override;
	virtual bool HasCancel() const override {return true;}
	
	void HandleAccept();
	void BakeToInstanceMesh(bool BakeSelected);
	void CreateNewAsset();
	void BuildCombinedMaterialSet(TArray<UMaterialInterface*>& NewMaterialsOut, TArray<TArray<int32>>& MaterialIDRemapsOut);
	void ApplyMethod(const TArray<AActor*>& Actors, UInteractiveToolManager* ToolManager, const AActor* MustKeepActor = nullptr);
	void HandleCancel();


private:
	
	UPROPERTY()
	TObjectPtr<class UPhysicsDropPropertySet> PropertySet;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedActors;

#pragma region DELEGATES

	FTestIfHitByClickDelegate TestHitByClickDelegate;
	FOnHitByClickDelegate OnHitByClickDelegate;
	FMouseBehaviorModiferCheckDelegate ModifierCheckDelegate;
	
#pragma endregion


#pragma region PHYSICAL TOOL LAYOUT

	/** Current layout mode */
	FString CurrentLayoutMode;
	
	/** Returns Previous layout mode */
	FString GetLastLayoutMode() const { return LastLayoutMode; }
	
	/** Previous layout mode */
	FString LastLayoutMode;

	/** Returns current layout mode */
	FString GetCurrentLayoutMode() const;

	/** Returns current layout mode text */
	FText GetCurrentLayoutModeText() const { return FText::FromString(CurrentLayoutMode); }

	virtual bool ShowModeWidgets() const;
	virtual bool UsesTransformWidget() const;
	virtual FVector GetWidgetLocation() const;

	/** Resets the selected actor's transform */
	void ResetTransform();
	
	/** Returns the spawned primitive components */
	TArray<UPrimitiveComponent*> GetSpawnedComponents();
	
	/** Returns the selected actors */
	TArray<AActor*> GetSelectedActors();
	
	
	/** Returns the spawned actors */
	TArray<AActor*> GetSpawnedActors() { 
		return SpawnedActors;
	}
	
	/** Destroys the all or selected actors */
	void DestroyActors(bool InSelected);
	
	/** Returns the selected primitive components */
	TArray<UPrimitiveComponent*> GetSelectedPrimitives();
	
	/** Unregisters the brush actor */
	void UnregisterBrush();
	
	/** Registers the brush actor */
	void RegisterBrush();
	
	/** Layout mode change event */
	void OnLayoutModeChange(FString InMode);
	
	/** Updates an actor's physics */
	void UpdatePhysics(AActor* InActor, bool bInEnableGravity);
	
	/** Makes the selected actors static */
	void MakeSelectedStatic();

	void SelectPlacedActors(UStaticMesh* InStaticMesh);

	void AddSelectedActor(AActor *InActor);
	void CachePhysics();

private:

	
	/** Brush actor's material */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BrushMI = nullptr;
	
	/** Brush actor */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Brush = nullptr;
	
	/** Is painting enable */
	bool bIsPainting = false;
	
	/** Returns the brush actor position */
	FVector GetPosition();
	
	/** Brush actor uniform size */
	float fBrushSize = 20;
	
	/** Cursor move world direction for brush actor */
	FVector BrushDirection;
	
	/** Brush actor position */
	FVector BrushPosition;
	
	/** Brush actor rotation */
	FVector BrushNormal;
	
	/** Cursor Last world position */
	FVector BrushLastPosition;
	
	/** Brush actor color */
	FLinearColor BrushColor = FLinearColor::Blue;
	
	/** Last place actor world position */
	FVector LastSpawnedPosition;
	
	/** Last selected actor list */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> LastSelectedActors;
	
	/** Last placed acotr list */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> LastSpawnedActors;
	
	
	/** All map's actors */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> LevelActors;
	
	/** Physics dictionary for all map's actors */
	UPROPERTY()
	TMap<TObjectPtr<UPrimitiveComponent>, bool> Physics;
	
	/** Gravity dictionary for all map's actors */
	UPROPERTY()
	TMap<TObjectPtr<UPrimitiveComponent>, bool> Gravities;
	
	/** Position dictionary for all map's actors */
	UPROPERTY()
	TMap<TObjectPtr<UPrimitiveComponent>, FVector> Positions;
	
	/** Rotation dictionary for all map's actors */
	UPROPERTY()
	TMap<TObjectPtr<UPrimitiveComponent>, FRotator> Rotations;
	
	/** Mobility dictionary for all map's actors */
	TMap<TObjectPtr<UPrimitiveComponent>, EComponentMobility::Type> Mobilities;
	
	/** Returns primitive components of an actor */
	TArray<UPrimitiveComponent*> GetPrimitives(const AActor* InActor);
	
	/** Updates the selected actor's physics */
	void UpdateSelectionPhysics();
	
	/** Resets the physics and transform for a primitive component */
	void ResetPrimitivePhysics(UPrimitiveComponent* InPrim, bool bResetTransform, bool bForceStatic=false);
	
	/** Traces the actor under cursor */
	virtual bool Trace(FHitResult& OutHit, const FInputDeviceRay& DevicePos) override;
	
	/** Resets all actors physics */
	void ResetPhysics();
		
	/** On Level actors added event */
	void OnLevelActorsAdded(AActor* InActor);
	
	/** On level actors deleted event */
	void OnLevelActorsDeleted(AActor* InActor);

	/** On Pre Begin Pie event */
	void OnPreBeginPie(bool InStarted);


	bool bSimulatePhysic = true;

private:
	FDelegateHandle OnLevelActorsAddedHandle;
	FDelegateHandle OnLevelActorsDeletedHandle;
	FDelegateHandle OnPreBeginPieHandle;
	
#pragma endregion  

	
};

UCLASS()
class HANDYMAN_API UPhysicsDropPropertySet : public UScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	//UPhysicsDropPropertySet();

	/* The list of meshes to drop by class. The float value is the chance of the item being dropped.
	 * The total of all the values should be 1.0
	 * If the total is not 1 the system will cull the last entry to make the total 1.0
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TArray<FReferenceMeshData> ItemsToDrop;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bDebugMeshPoints = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "0.1"))
	float NormalDistance = 20.0f;

	/** Minimum distance to last placed actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "0.1"))
	float MinDistance = 2;
	
	/** Minimum position random */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FVector MinPositionRandom = FVector::ZeroVector;
	
	/** Maximum position random */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FVector MaxPositionRandom = FVector::ZeroVector;

	/** Minimum rotation random */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FRotator MinRotateRandom = FRotator::ZeroRotator;
	
	/** Maximum rotation random */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FRotator MaxRotateRandom = FRotator::ZeroRotator;

	/** Minimum scale random */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FVector MinScaleRandom = FVector::OneVector;
	
	/** Maximum scale random */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FVector MaxScaleRandom = FVector::OneVector;

	/** Is select placed actor enable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bSelectPlacedActors = true;
	
	/** Is minimum scale lock */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bMinScaleLock = false;
	
	/** Is maximum scale lock */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bMaxScaleLock = false;
	
	/** Is gravity enable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bEnableGravity = true;
	
	/** Is use selected enable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bUseSelected = false;


	FString GetLayoutMode() const { return LayoutMode; }
	
	
	
	/** Returns the current actor to place */
	UStaticMesh* GetRandomMesh() const {return PickedMesh;};

	/** Changes the layout mode by name */
	void ChangeMode(FString InLayoutMode);
	
	/** Changes the layout mode by index */
	void ChangeMode(int InDirection);
	
	/** Returns true if we are selecting the placed actor  */
	bool IsSelectingPlacedActors() const { return bSelectPlacedActors; }
	
	/** Returns true if minimum scale random is lock */
	bool IsMinScaleLock () const { return bMinScaleLock; }
	
	/** Returns true if maximum scale random is lock */
	bool IsMaxScaleLock () const { return bMaxScaleLock; }
	
	/** Returns true if gravity is enable */
	bool IsEnableGravity () const { return bEnableGravity; }
	
	/** Returns true if use selectd is enable */
	bool IsUseSelected () const { return bUseSelected; }
	
	/** Returns normal distance for hited polygon */
	float GetNormalDistance () const { return fNormalDistance; }
	
	/** Returns position randomness */
	FVector GetPositionRandom() const { return PositionRandom; }
	
	/** Returns rotatin randomness */
	FRotator GetRotateRandom() const { return RotateRandom; }
	
	/** Returns scale randomness */
	FVector GetScaleRandom() const { return ScaleRandom; }
	
	/** Returns normal rotation offset */
	FRotator GetNormalRotation() const { return NormalRotation; }

	/** Sets the next actor to place */
	void SetRandomMesh();
	
	/** Returns true if damping velocity */
	bool IsDamplingVelocity() const
	{ return bDampVelocity; }

	/** Sets Damp Velocity */
	void SetDampVelocity(bool InDampVelocity)
	{ bDampVelocity = InDampVelocity; }

	void LoadPLPreset(const FAssetData& InAsset);
private:

	/** Is Velocity Getting Damp */
	bool bDampVelocity;

	/** Previous layout mode */
	FString LastLayoutMode;
	
	/** Position random */
	FVector PositionRandom = FVector::ZeroVector;
	
	/** Rotation random */
	FRotator RotateRandom = FRotator::ZeroRotator;
	
	/** Scale random */
	FVector ScaleRandom = FVector::OneVector;

	/** Nornaml rotation offset */
	FRotator NormalRotation = FRotator::ZeroRotator;

	/** Normal distance for hited polygon */
	float fNormalDistance = 0;

	/** Selected reference mesh index */
	int SelectedMeshIndex = -1;
	
	
	

	/** Layout mode name */
	FString LayoutMode = ELayoutMode::Paint;
	
	/** Is percents are relative */
	bool bIsPercentRelative;
	
	/** Layout mode names */
	TArray<TSharedPtr<FString>> LayoutModes;
	
	/** Reference mesh thumbnail pool */
	TSharedPtr<FAssetThumbnailPool> ThumbnailPool;
	
	/** Bakes the places actors into instance mesh */
	void BakeToInstanceMesh(bool BakeSelected);
	
	/** Returns reference meshes */
	TArray<FReferenceMeshData> GetReferenceMeshes() const;
	
	UPROPERTY()
	TObjectPtr<UStaticMesh> PickedMesh = nullptr;

	int32 LastReferenceMeshCount = 0;

	

	
};

