// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingGeneratorTool.h"

#include "ActorGroupingUtils.h"
#include "ModelingToolTargetUtil.h"
#include "BaseGizmos/CombinedTransformGizmo.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ToolSet/HandyManTools/PCG/BuildingGenerator/Actor/PCG_BuildingGenerator.h"

#define LOCTEXT_NAMESPACE "BuildingGeneratorTool"

UBuildingGeneratorTool::UBuildingGeneratorTool()
{
	ToolName = LOCTEXT("BuildingGeneratorToolName", "Building Generator");
	ToolTooltip = LOCTEXT("BuildingGeneratorToolDescription", "Generate buildings from an input mesh");
	ToolLongName = LOCTEXT("BuildingGeneratorToolLongName", "Generate Building From Blockout");
	ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
	ToolStartupRequirements = EScriptableToolStartupRequirements::ToolTarget;
}

void UBuildingGeneratorTool::CreateBrush()
{
	UMaterial* BrushMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EditorLandscapeResources/FoliageBrushSphereMaterial.FoliageBrushSphereMaterial"), nullptr, LOAD_None, nullptr);
	BrushMI = UMaterialInstanceDynamic::Create(BrushMaterial, GetTransientPackage());
	BrushMI->SetVectorParameterValue(TEXT("HighlightColor"), FLinearColor::Red);
	check(BrushMI != nullptr);

	Brush = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("SphereBrushComponent"));
	Brush->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Brush->SetCollisionObjectType(ECC_WorldDynamic);
	Brush->SetMaterial(0, BrushMI);
	Brush->SetAbsolute(true, true, true);
	Brush->CastShadow = false;
}

void UBuildingGeneratorTool::SpawnOutputActorInstance(const UBuildingGeneratorPropertySet* InSettings, const FTransform& SpawnTransform)
{
	if (GetHandyManAPI() && InSettings && TargetActor && (TargetActor->IsA<AStaticMeshActor>() || TargetActor->IsA<ADynamicMeshActor>()))
	{

		// Generate the splines from the input actor
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags = RF_Transactional;
		SpawnInfo.Name = FName("SplineActor");

		auto World = GetToolWorld();
		auto ClassToSpawn = GetHandyManAPI()->GetPCGActorClass(FName(ToolName.ToString()));
		if (auto SpawnedActor =  World->SpawnActor<APCG_BuildingGenerator>(ClassToSpawn))
		{
			// Initalize the actor
			SpawnedActor->SetActorTransform(SpawnTransform);
			
			if (InSettings->bUseConsistentFloorMaterial)
			{
				SpawnedActor->SetFloorMaterial(0, InSettings->FloorMaterial);
			}
			else
			{
				for (const auto Mat : InSettings->FloorMaterialMap)
				{
					SpawnedActor->SetFloorMaterial(Mat.Key, Mat.Value);
				}
			}
			
			SpawnedActor->SetBuildingMaterial(InSettings->BuildingMaterial);
			SpawnedActor->SetWallThickness(InSettings->WallThickness);
			SpawnedActor->SetNumberOfFloors(InSettings->DesiredNumberOfFloors);
			SpawnedActor->SetHasOpenRoof(InSettings->bHasOpenRoof);
			
			SpawnedActor->CacheInputActor(TargetActor);
			
			TargetPCGInterface = CastChecked<IPCGToolInterface>(SpawnedActor);

			if (TargetPCGInterface.IsValid())
			{
				TargetPCGInterface.Get()->GetPCGComponent()->GenerateLocal(true);
			}
			
			OutputActor = (SpawnedActor);
		}
		
	}
	else
	{
		// TODO : Error Dialogue
	}

}

void UBuildingGeneratorTool::OnLevelActorsAdded(AActor* InActor)
{
	if (InActor && InActor->IsA<AStaticMeshActor>() && !InActor->IsActorBeingDestroyed())
	{
		if (InActor->GetLevel() == GetWorld()->GetCurrentLevel())
		{
			// When the player drags a mesh into the world, like a door or window this function should set the brush mesh to this mesh
			// Should also cache the mesh as the active painting mesh.

			PaintingMesh = Cast<AStaticMeshActor>(InActor)->GetStaticMeshComponent()->GetStaticMesh();
			
			if (IsValid(Brush))
			{
				if (!Brush->IsRegistered())
				{
					Brush->RegisterComponentWithWorld(GetWorld());
				}
				Brush->SetStaticMesh(PaintingMesh);
			}
		}

		InActor->Destroy();
	}
}

void UBuildingGeneratorTool::OnLevelActorsDeleted(AActor* InActor)
{
	
}

void UBuildingGeneratorTool::Setup()
{
	Super::Setup();

	if (Targets.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
			LOCTEXT("UBuildingGeneratorTool", "You do not have a target actor selected. Please select either an AStaticMeshActor or ADynamicMeshActor as the target actor."));
		
		return;
	}
	
	CreateBrush();

	EToolsFrameworkOutcomePins PropertyCreationOutcome;
	Settings = Cast<UBuildingGeneratorPropertySet>(AddPropertySetOfType(UBuildingGeneratorPropertySet::StaticClass(), "Settings", PropertyCreationOutcome));

	TargetActor = UE::ToolTarget::GetTargetActor(Targets[0]);

	

	Settings->WatchProperty(Settings->BuildingMaterial, [this](TSoftObjectPtr<UMaterialInterface>)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetBuildingMaterial(Settings->BuildingMaterial);
		}
	});
	
	Settings->WatchProperty(Settings->DesiredBuildingHeight, [this](float)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetDesiredBuildingHeight(Settings->DesiredBuildingHeight);
			
		}
	});
	
	Settings->WatchProperty(Settings->BaseFloorHeight, [this](float)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetBaseFloorHeight(Settings->BaseFloorHeight);
		}
	});
	Settings->WatchProperty(Settings->DesiredFloorClearance, [this](float)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetDesiredFloorClearance(Settings->DesiredFloorClearance);
		}
	});
	Settings->WatchProperty(Settings->WallThickness, [this](float)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetWallThickness(Settings->WallThickness);
		}
	});
	
	Settings->WatchProperty(Settings->DesiredNumberOfFloors, [this](int32)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetNumberOfFloors(Settings->DesiredNumberOfFloors);
		}
	});
	
	Settings->WatchProperty(Settings->bHasOpenRoof, [this](bool)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetHasOpenRoof(Settings->bHasOpenRoof);
		}
	});
	
	Settings->WatchProperty(Settings->bUseConsistentFloorHeight, [this](bool)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetUseConsistentFloorHeight(Settings->bUseConsistentFloorHeight);
			for (const auto Mat : Settings->FloorMaterialMap)
			{
				OutputActor->SetFloorMaterial(Mat.Key, Mat.Value);
			}
		}
	});
	
	Settings->WatchProperty(Settings->bUseConsistentFloorMaterial, [this](bool)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetUseConsistentFloorMaterials(Settings->bUseConsistentFloorMaterial);
		}
	});
	
	
	Settings->SilentUpdateWatched();


	if (TargetActor->IsA(APCG_BuildingGenerator::StaticClass()))
	{
		bIsEditing = true;
		OutputActor = Cast<APCG_BuildingGenerator>(TargetActor);
		
		GEditor->SelectActor(TargetActor, false, true);

		// Spawn all gizmos for the array of FGeneratedOpening
		for (const auto& Opening : OutputActor->GetGeneratedOpenings())
		{
			if(!Opening.Mesh) continue;
			
			FScriptableToolGizmoOptions GizmoOptions;
			GizmoOptions.bAllowNegativeScaling = false;
			CreateTRSGizmo(Opening.Mesh->GetFName().ToString(), Opening.Mesh->GetActorTransform(), GizmoOptions, PropertyCreationOutcome);

			CachedOpenings.Openings.Add(Opening);
			EditedOpenings.Openings.Add(Opening);
			LastSpawnedActors.Add(Opening.Mesh);
			Opening.Mesh->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		}


		HideAllGizmos();

		if (Settings)
		{
			// Restore any settings set from the tool back into the tool.
			for (const auto& Item : OutputActor->GetGeneratedOpeningsMap())
			{
				FDynamicOpening Opening;
				Opening.Mesh = Item.Key;
				Opening.BooleanShape = Item.Value.Openings[0].BooleanShape;
				Opening.bIsSubtractiveBoolean = Item.Value.Openings[0].bShouldCutHoleInTargetMesh;
				Opening.bShouldApplyBoolean = Item.Value.Openings[0].bShouldApplyBoolean;
				Opening.Fit = Item.Value.Openings[0].Fit;
				Opening.bShouldSnapToGroundSurface = Item.Value.Openings[0].bShouldSnapToGroundSurface;
				Settings->Openings.Add(Opening);
			}

			// TODO : Fill out the rest of the settings from the actor - NEEDS GETTER FUNCTIONS -

		}

		
	}
	else
	{
		SpawnOutputActorInstance(Settings, TargetActor->GetActorTransform());
	}

	

	
}

FInputRayHit UBuildingGeneratorTool::TestCanHoverFunc_Implementation(const FInputDeviceRay& PressPos, const FScriptableToolModifierStates& Modifiers)
{
	return UScriptableToolsUtilityLibrary::MakeInputRayHit_MaxDepth();
}


void UBuildingGeneratorTool::OnBeginHover(const FInputDeviceRay& DevicePos, const FScriptableToolModifierStates& Modifiers)
{
}

bool UBuildingGeneratorTool::UpdateBrush(const FInputDeviceRay& DevicePos)
{
	FHitResult Hit;
	const bool bWasHit = Trace(Hit, DevicePos);

	auto Rotation = (Hit.Normal).GetSafeNormal().Rotation();
	auto PickedMesh = PaintingMesh;
	if (IsValid(PickedMesh))
	{
		if (!IsValid(Brush))
		{
			return false;
		}
		
		if (!Brush->IsRegistered())
		{
			Brush->RegisterComponentWithWorld(GetWorld());
		}

		if (PickedMesh != Brush->GetStaticMesh())
		{
			Brush->SetStaticMesh(PickedMesh);
		}

		
		Brush->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));

		if (!bIsSnapping)
		{
			Brush->SetWorldLocation(Hit.Location);
			Brush->SetWorldRotation(Rotation);
		}
		else
		{
			Brush->SetWorldRotation(SnapStartRotation);
		}
	}

	BrushMI->SetVectorParameterValue(TEXT("HighlightColor"), bWasHit ? FLinearColor::Green : FLinearColor::Red);
	return bWasHit;
}

bool UBuildingGeneratorTool::OnUpdateHover(const FInputDeviceRay& DevicePos, const FScriptableToolModifierStates& Modifiers)
{
	return UpdateBrush(DevicePos);
}

void UBuildingGeneratorTool::OnEndHover()
{
}

FInputRayHit UBuildingGeneratorTool::CanClickDrag_Implementation(const FInputDeviceRay& PressPos,
	const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button)
{
	return UScriptableToolsUtilityLibrary::MakeInputRayHit_MaxDepth();
}


void UBuildingGeneratorTool::OnClickDrag(const FInputDeviceRay& DragPos, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button)
{
	LatestPosition = BrushPosition;
	

	


	switch (Button)
	{
	case EScriptableToolMouseButton::LeftButton:
		{
			FHitResult Hit;
			const bool bWasHit = Trace(Hit, DragPos);
			UpdateBrush(DragPos);
			Brush->SetVisibility(true);
			
			HideAllGizmos();
			
		}
		
		break;
	case EScriptableToolMouseButton::RightButton:
		break;
	case EScriptableToolMouseButton::MiddleButton:
		{
			// Check if hit actor is a static mesh actor with the same mesh as our painting mesh
			// If it is, delete the actor
			//GetToolManager()->BeginUndoTransaction(LOCTEXT("DeleteActor", "Delete Opening"));

			TArray<FHitResult> OutHits;
			const bool bWasHit = Trace(OutHits, DragPos);

			for (const auto& Hit : OutHits)
			{
				if (Hit.GetActor() && Hit.GetActor()->IsA(AStaticMeshActor::StaticClass())
				&& Cast<AStaticMeshActor>(Hit.GetActor())->GetStaticMeshComponent()->GetStaticMesh()
				&& LastSpawnedActors.Contains(Hit.GetActor()))
				{
				
					const FName ActorName = Hit.GetActor()->GetFName();
					EToolsFrameworkOutcomePins OutCome;
					DestroyTRSGizmo(ActorName.ToString(), OutCome);

					if (!bIsEditing)
					{
						LastSpawnedActors.RemoveSingle(Hit.GetActor());
						Hit.GetActor()->Destroy();
					
						for (int i = 0; i < CachedOpenings.Openings.Num(); ++i)
						{
							if(!CachedOpenings.Openings[i].Mesh.GetFName().IsEqual(ActorName)) continue;
							CachedOpenings.Openings.RemoveAt(i);
							break;
						}

						if (OutputActor)
						{
							OutputActor->UpdatedGeneratedOpenings(CachedOpenings.Openings);
						}
					}
					else
					{
						SpawnedActorsToDestroy.Add(Hit.GetActor());
						Hit.GetActor()->SetIsTemporarilyHiddenInEditor(true);
						for (int i = 0; i < EditedOpenings.Openings.Num(); ++i)
						{
							if(!EditedOpenings.Openings[i].Mesh.GetFName().IsEqual(ActorName)) continue;
							EditedOpenings.Openings.RemoveAt(i);
							break;
						}

						if (OutputActor)
						{
							OutputActor->UpdatedGeneratedOpenings(EditedOpenings.Openings);
						}
					}
				
				}
			}
		}

		//GetToolManager()->EndUndoTransaction();
		break;
	}
}

void UBuildingGeneratorTool::OnDragBegin(const FInputDeviceRay& StartPosition, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button)
{

	bIsPainting = true;
	
	auto PickedMesh = PaintingMesh;
	FHitResult Hit;
	const bool bWasHit = Trace(Hit, StartPosition);
	

	if (Button == EScriptableToolMouseButton::LeftButton)
	{
		if (IsValid(PickedMesh))
		{
			if (!IsValid(Brush))
			{
				return;
			}


			// If we are holding a modifier key cache the last hovered opening
			if (!bWasHit && (IsShiftDown() || IsCtrlDown()))
			{
				if (!bIsEditing)
				{
					if (Hit.GetActor() && CachedOpenings.Contains(Hit.GetActor()))
					{
						LastHoveredOpening = Hit.GetActor();
					}
				}
				else
				{
					if (Hit.GetActor() && EditedOpenings.Contains(Hit.GetActor()))
					{
						LastHoveredOpening = Hit.GetActor();
					}
				}

			}
			else if (bWasHit && !(IsShiftDown() || IsCtrlDown()))
			{
				LastHoveredOpening = nullptr;
			}
			

			const auto Extent = PickedMesh->GetBounds().BoxExtent;
				
			Brush->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));

			if (LastHoveredOpening)
			{
				if (IsShiftDown() || IsCtrlDown())
				{
					if (!bIsSnapping)
					{
						bIsSnapping = true;
						SnapStartLocation = LastHoveredOpening->GetActorLocation();
						SnapStartRotation = LastHoveredOpening->GetActorRotation();
						Brush->SetWorldLocation(SnapStartLocation);
						Brush->AddLocalOffset(FVector(Extent.X, 0, 0));
					}
				}
				else
				{
					bIsSnapping = false;
					SnapStartLocation = FVector::ZeroVector;
					SnapStartRotation = FRotator::ZeroRotator;
					Brush->SetWorldLocation(Hit.Location);
				}

				if (IsShiftDown() && !IsCtrlDown())
				{
					SnappingDirection = FVector(0, 1, 0);
					bIsSnappingOnRightAxis = true;
				}

				if (!IsShiftDown() && IsCtrlDown())
				{
					SnappingDirection = FVector(0, 0, 1);
					bIsSnappingOnRightAxis = false;
				}
			}
			else
			{
				Brush->SetWorldLocation(Hit.Location);
			}
			
			LastFrameScreenPosition = StartPosition.ScreenPosition;
		}
	
	}
	
}

void UBuildingGeneratorTool::SpawnOpeningFromReference(FDynamicOpening OpeningRef, UE::Math::TRotator<double> Rotation)
{
	// If our Mesh is a static mesh actor then we need to spawn a static mesh actor else its just an actor and we should spawn it as is.
	UClass* ClassToSpawn = OpeningRef.Mesh.IsA(UStaticMesh::StaticClass()) ? AStaticMeshActor::StaticClass() : OpeningRef.Mesh.GetClass();

	FActorSpawnParameters Params = FActorSpawnParameters();
	FString name = FString::Format(TEXT("Actor_{0}"), { PaintingMesh->GetFName().ToString() });
	FName fname = MakeUniqueObjectName(nullptr, ClassToSpawn, FName(*name));
	Params.Name = fname;
	Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;

	AActor* actor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, Brush->GetComponentLocation(), Rotation, Params);
	actor->GetRootComponent()->SetMobility(EComponentMobility::Type::Movable);
		

	actor->SetActorLabel(fname.ToString());
	LastSpawnedActors.Add(actor);
	if (actor->IsA(AStaticMeshActor::StaticClass()))
	{
		Cast<AStaticMeshActor>(actor)->GetStaticMeshComponent()->SetStaticMesh(PaintingMesh);
	}


	const FVector Extent = PaintingMesh->GetBounds().BoxExtent;
	float Offset = 0.f;
	FVector MeshScale = FVector::OneVector;
	switch (OpeningRef.Fit)
	{
	case EMeshFitStyle::Flush:
		{
			Offset = -Extent.X;
			const float CurrentX = (Extent.X * 2);
			if (Settings->WallThickness > CurrentX)
			{
				MeshScale = FVector(Settings->WallThickness / CurrentX, 1.0f, 1.0f);
			}
		}
		break;
	case EMeshFitStyle::In_Front:
		Offset = Extent.X;
		break;
	case EMeshFitStyle::Centered:
		{
			Offset = -Extent.X;
			const float CurrentX = (Extent.X * 2);
			if (!FMath::IsNearlyEqual(Settings->WallThickness, CurrentX))
			{
				const auto XScale = (Settings->WallThickness / CurrentX) + (OpeningRef.CenteredOffset * .01f);
				MeshScale = FVector(XScale, 1.0f, 1.0f);
			}
			else
			{
				MeshScale = FVector(1.0f + (OpeningRef.CenteredOffset * .01f), 1.0f, 1.0f);
			}
		}
		break;
	}
			
	const FTransform SpawnTransform = FTransform(Rotation, Brush->GetComponentLocation(), FVector::One());
	actor->SetActorTransform(SpawnTransform);
	actor->GetRootComponent()->SetRelativeScale3D(MeshScale);
	actor->GetRootComponent()->AddLocalOffset(FVector(Offset, 0,0));

			

	FGeneratedOpening NewOpening;
	NewOpening.Transform = actor->GetActorTransform();
	NewOpening.Mesh = actor;
	NewOpening.bShouldCutHoleInTargetMesh = OpeningRef.bIsSubtractiveBoolean;
	NewOpening.bShouldApplyBoolean = OpeningRef.bShouldApplyBoolean;
	NewOpening.bShouldSnapToGroundSurface = OpeningRef.bShouldSnapToGroundSurface;
	NewOpening.BooleanShape = OpeningRef.BooleanShape;
	NewOpening.Fit = OpeningRef.Fit;
	NewOpening.CenteredOffset = OpeningRef.CenteredOffset;
			
	if (OutputActor)
	{
		OutputActor->AddGeneratedOpeningEntry(NewOpening);
	}

	if (!bIsEditing)
	{
		CachedOpenings.Openings.Add(NewOpening);
	}
	else
	{
		EditedOpenings.Openings.Add(NewOpening);
		EditModeSpawnedActors.Add(actor);
	}

	// Add a gizmo so in edit mode the user can move without needing to use the modifiers.
	EToolsFrameworkOutcomePins OutCome;
	FScriptableToolGizmoOptions GizmoOptions;
	GizmoOptions.bAllowNegativeScaling = false;
			
	CreateTRSGizmo(actor->GetFName().ToString(), actor->GetActorTransform(), GizmoOptions, OutCome);
	SetGizmoVisible(actor->GetFName().ToString(), false);
}

void UBuildingGeneratorTool::SpawnOpeningFromReference(FGeneratedOpening OpeningRef, bool bSetGizmoActive)
{
	// If our Mesh is a static mesh actor then we need to spawn a static mesh actor else its just an actor and we should spawn it as is.
	UClass* ClassToSpawn = OpeningRef.Mesh.IsA(UStaticMesh::StaticClass()) ? AStaticMeshActor::StaticClass() : OpeningRef.Mesh.GetClass();
	const auto Rotation = OpeningRef.Mesh->GetActorRotation();
	FString MeshName = "";
	if (OpeningRef.Mesh->FindComponentByClass<UStaticMeshComponent>())
	{
		PaintingMesh = OpeningRef.Mesh->FindComponentByClass<UStaticMeshComponent>()->GetStaticMesh();
		MeshName = PaintingMesh->GetFName().ToString();
	}
	FActorSpawnParameters Params = FActorSpawnParameters();
	FString name = FString::Format(TEXT("Actor_{0}"), { MeshName });
	FName fname = MakeUniqueObjectName(nullptr, ClassToSpawn, FName(*name));
	Params.Name = fname;
	Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;

	AActor* actor = GetWorld()->SpawnActor<AActor>(OpeningRef.Mesh.GetClass(), Brush->GetComponentLocation(), Rotation, Params);
	actor->GetRootComponent()->SetMobility(EComponentMobility::Type::Movable);
		

	actor->SetActorLabel(fname.ToString());
	LastSpawnedActors.Add(actor);
	if (actor->IsA(AStaticMeshActor::StaticClass()))
	{
		Cast<AStaticMeshActor>(actor)->GetStaticMeshComponent()->SetStaticMesh(PaintingMesh);
	}

	
			
	const FTransform SpawnTransform = OpeningRef.Transform;
	actor->SetActorTransform(SpawnTransform);

			

	FGeneratedOpening NewOpening;
	NewOpening.Transform = actor->GetActorTransform();
	NewOpening.Mesh = actor;
	NewOpening.bShouldCutHoleInTargetMesh = OpeningRef.bShouldCutHoleInTargetMesh;
	NewOpening.bShouldApplyBoolean = OpeningRef.bShouldApplyBoolean;
	NewOpening.bShouldSnapToGroundSurface = OpeningRef.bShouldSnapToGroundSurface;
	NewOpening.BooleanShape = OpeningRef.BooleanShape;
	NewOpening.Fit = OpeningRef.Fit;
	NewOpening.CenteredOffset = OpeningRef.CenteredOffset;
			
	if (OutputActor)
	{
		OutputActor->AddGeneratedOpeningEntry(NewOpening);
	}

	if (!bIsEditing)
	{
		CachedOpenings.Openings.Add(NewOpening);
	}
	else
	{
		EditedOpenings.Openings.Add(NewOpening);
		EditModeSpawnedActors.Add(actor);
	}

	// Add a gizmo so in edit mode the user can move without needing to use the modifiers.
	EToolsFrameworkOutcomePins OutCome;
	FScriptableToolGizmoOptions GizmoOptions;
	GizmoOptions.bAllowNegativeScaling = false;
			
	CreateTRSGizmo(actor->GetFName().ToString(), actor->GetActorTransform(), GizmoOptions, OutCome);
	SetGizmoVisible(actor->GetFName().ToString(), bSetGizmoActive);
}

void UBuildingGeneratorTool::OnDragEnd(const FInputDeviceRay& EndPosition, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button)
{
	if (!bIsPainting)
	{
		return;
	}

	if (Button == EScriptableToolMouseButton::LeftButton)
	{
		if (!PaintingMesh)
		{
			FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
				LOCTEXT("UBuildingGeneratorTool", "You are trying to add meshes to the building but no mesh is selected. Please select a mesh by dragging it into the scene from the asset or content browser."));
			return;
		}
			
		FHitResult Hit;
		const bool bWasHit = Trace(Hit, EndPosition);
		
		if (bWasHit && IsValid(PaintingMesh))
		{

			FDynamicOpening OpeningRef;

			for (int i = 0; i < Settings->Openings.Num(); ++i)
			{
				UObject* NextMesh = Settings->Openings[i].Mesh;

				if (!IsValid(NextMesh))
				{
					return;
				}

				if (NextMesh->IsA(AActor::StaticClass()))
				{
					TArray<UActorComponent*> ComponentsArray = Cast<AActor>(NextMesh)->K2_GetComponentsByClass(UStaticMeshComponent::StaticClass());

					if (IsValid(ComponentsArray[0]) && IsValid(Cast<UStaticMeshComponent>(ComponentsArray[0])->GetStaticMesh()))
					{
						if (Cast<UStaticMeshComponent>(ComponentsArray[0])->GetStaticMesh() == PaintingMesh)
						{
							OpeningRef = Settings->Openings[i];
							break;
						}
					}
				}
				else if (NextMesh->IsA(UStaticMesh::StaticClass()))
				{
					if (Cast<UStaticMesh>(NextMesh) == PaintingMesh)
					{
						OpeningRef = Settings->Openings[i];
						break;
					}
				}
			}
		
			bIsPainting = true;

			LastSpawnedPosition = Hit.Location;
			auto Rotation = Hit.Normal.Rotation();

			
			SpawnOpeningFromReference(OpeningRef, Rotation);
		}

		ResetBrush();
	}
}

void UBuildingGeneratorTool::HideAllGizmos()
{
	for (const auto& Gizmo : Gizmos)
	{
		Gizmo.Value->SetVisibility(false);
	}
}

void UBuildingGeneratorTool::ResetBrush()
{
	Brush->SetVisibility(true);
	CopiedScale = FVector::Zero();
	LastHoveredOpening = nullptr;
	bIsSnapping = false;
	SnapStartLocation = FVector::ZeroVector;
	SnapStartRotation = FRotator::ZeroRotator;
	bIsCopying = false;
	HideAllGizmos();
}

FInputRayHit UBuildingGeneratorTool::CanClickFunc_Implementation(const FInputDeviceRay& PressPos, const EScriptableToolMouseButton& Button)
{
	if (!IsShiftDown() && !IsCtrlDown())
	{
		return UScriptableToolsUtilityLibrary::MakeInputRayHit_Miss();
	}

	
	
	FVector Start = PressPos.WorldRay.Origin;
	FVector End = PressPos.WorldRay.Origin + PressPos.WorldRay.Direction * HALF_WORLD_MAX;
	FHitResult Hit;

	if (GetToolWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		if (IsValid(Hit.GetActor()))
		{
			if (Hit.GetActor()->IsA(APCG_BuildingGenerator::StaticClass()))
			{
				ResetBrush();
				
				return UScriptableToolsUtilityLibrary::MakeInputRayHit(Hit.Distance, nullptr);
			}

			
			bool bHasHitAnOpening = false;
			if (!bIsEditing)
			{
				for (const auto& Opening : CachedOpenings.Openings)
				{
					if(!Opening.Mesh.GetFName().IsEqual(Hit.GetActor()->GetFName())) continue;
					bHasHitAnOpening = true;
				}
			}
			else
			{
				for (const auto& Opening : EditedOpenings.Openings)
				{
					if(!Opening.Mesh.GetFName().IsEqual(Hit.GetActor()->GetFName())) continue;
					bHasHitAnOpening = true;
				}
			}

			if (bHasHitAnOpening)
			{
				bIsCopying = false;
				return UScriptableToolsUtilityLibrary::MakeInputRayHit(Hit.Distance, nullptr);
			}
		}
	}
	
	return UScriptableToolsUtilityLibrary::MakeInputRayHit_Miss();

	
}

void UBuildingGeneratorTool::OnHitByClickFunc(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& MouseButton)
{
	FVector Start = ClickPos.WorldRay.Origin;
	FVector End = ClickPos.WorldRay.Origin + ClickPos.WorldRay.Direction * HALF_WORLD_MAX;
	FHitResult Hit;

	if (GetToolWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		if (IsValid(Hit.GetActor()))
		{
			FString Identifier = "";
			if (!bIsEditing)
			{
				for (const auto& Opening : CachedOpenings.Openings)
				{
					if(!Opening.Mesh.GetFName().IsEqual(Hit.GetActor()->GetFName())) continue;
					Identifier = Hit.GetActor()->GetFName().ToString();
				}
			}
			else
			{
				for (const auto& Opening : EditedOpenings.Openings)
				{
					if(!Opening.Mesh.GetFName().IsEqual(Hit.GetActor()->GetFName())) continue;
					Identifier = Hit.GetActor()->GetFName().ToString();
				}
			}

			if (Identifier.IsEmpty()) {return;}

			GetToolManager()->BeginUndoTransaction(LOCTEXT("EditOpening", "Edit Opening"));
			for (const auto& Gizmo : Gizmos)
			{
				if(!Identifier.Equals(Gizmo.Key))
				{
					Gizmo.Value->SetVisibility(false);
				}
				else
				{
					if (IsCtrlDown())
					{
						if (CopiedScale.Equals(FVector::Zero()) || IsShiftDown())
						{
							CopiedScale = Hit.GetActor()->GetActorRelativeScale3D();
							Brush->SetWorldScale3D(CopiedScale);
							GetToolManager()->DisplayMessage(LOCTEXT("CopiedScaleName", "Copied Scale"), EToolMessageLevel::UserMessage);
							return;
						}

						Hit.GetActor()->SetActorRelativeScale3D(CopiedScale);
						OutputActor->RerunConstructionScripts();
						Brush->SetWorldScale3D(FVector::One());
						GetToolManager()->DisplayMessage(LOCTEXT("PastedScaleName", "Pasted Scale"), EToolMessageLevel::UserMessage);
						return;
					}
					
					Gizmo.Value->SetVisibility(true);
					// Turn off the brush so the user can see the gizmo
					Brush->SetVisibility(false);
				}
			}

			GetToolManager()->EndUndoTransaction();
		}
	}
}

bool UBuildingGeneratorTool::MouseBehaviorModiferCheckFunc(const FInputDeviceState& InputDeviceState)
{
	return true;
}

FInputRayHit UBuildingGeneratorTool::CanUseMouseWheel_Implementation(const FInputDeviceRay& PressPos)
{
	// Only return a hit if the player is holding the shift modifier key else let them use the scroll how they would naturally

	if (IsShiftDown())
	{
		return UScriptableToolsUtilityLibrary::MakeInputRayHit_MaxDepth();
	}

	return FInputRayHit();
}

void UBuildingGeneratorTool::OnMouseWheelUp(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	// Get the current index of the selected mesh and increment it by 1 to select the next mesh
	// If this is the last mesh in the array then loop back to the first mesh

	if (Settings->Openings.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
			LOCTEXT("UBuildingGeneratorTool", "You do not have any openings set up in the settings. Please add some openings to the settings."));
		return;
	}

	int32 NextIndex = LastOpeningIndex + 1;
	
	if (NextIndex == Settings->Openings.Num())
	{
		NextIndex = 0;
	}

	LastOpeningIndex = NextIndex;

	// Get the mesh and spawn it as the type of actor necessary
	UObject* NextMesh = Settings->Openings[NextIndex].Mesh;

	if (!IsValid(NextMesh))
	{
		return;
	}

	if (NextMesh->IsA(AActor::StaticClass()))
	{
		// Get The static mesh component and set the brushes mesh to the static mesh
		TArray<UActorComponent*> ComponentsArray = Cast<AActor>(NextMesh)->K2_GetComponentsByClass(UStaticMeshComponent::StaticClass());

		if (IsValid(ComponentsArray[0]) && IsValid(Cast<UStaticMeshComponent>(ComponentsArray[0])->GetStaticMesh()))
		{
			PaintingMesh = Cast<UStaticMeshComponent>(ComponentsArray[0])->GetStaticMesh();
			Brush->SetStaticMesh(PaintingMesh);
			Brush->SetWorldScale3D(FVector::One());
		}
	}
	else if (NextMesh->IsA(UStaticMesh::StaticClass()))
	{
		// Get The static mesh and set the brushes mesh to the static mesh
		PaintingMesh = Cast<UStaticMesh>(NextMesh);
		Brush->SetStaticMesh(PaintingMesh);
		Brush->SetWorldScale3D(FVector::One());
	}
	else
	{
		// Error Dialogue
	}
	
}

void UBuildingGeneratorTool::OnMouseWheelDown(const FInputDeviceRay& ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	if (Settings->Openings.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
			LOCTEXT("UBuildingGeneratorTool", "You do not have any openings set up in the settings. Please add some openings to the settings."));
		return;
	}

	int32 NextIndex = LastOpeningIndex - 1;
	
	if (NextIndex < 0)
	{
		NextIndex = Settings->Openings.Num() - 1;
	}

	LastOpeningIndex = NextIndex;

	// Get the mesh and spawn it as the type of actor necessary
	UObject* NextMesh = Settings->Openings[NextIndex].Mesh;

	if (!IsValid(NextMesh))
	{
		return;
	}

	if (NextMesh->IsA(AActor::StaticClass()))
	{
		// Get The static mesh component and set the brushes mesh to the static mesh
		TArray<UActorComponent*> ComponentsArray = Cast<AActor>(NextMesh)->K2_GetComponentsByClass(UStaticMeshComponent::StaticClass());

		if (IsValid(ComponentsArray[0]) && IsValid(Cast<UStaticMeshComponent>(ComponentsArray[0])->GetStaticMesh()))
		{
			PaintingMesh = Cast<UStaticMeshComponent>(ComponentsArray[0])->GetStaticMesh();
			Brush->SetStaticMesh(PaintingMesh);
			Brush->SetWorldScale3D(FVector::One());
		}
	}
	else if (NextMesh->IsA(UStaticMesh::StaticClass()))
	{
		// Get The static mesh and set the brushes mesh to the static mesh
		PaintingMesh = Cast<UStaticMesh>(NextMesh);
		Brush->SetStaticMesh(PaintingMesh);
		Brush->SetWorldScale3D(FVector::One());
	}
	else
	{
		// Error Dialogue
	}
}

void UBuildingGeneratorTool::UpdateOpeningTransforms(const FString& GizmoIdentifier, const FTransform& CurrentTransform)
{
	if (!bIsEditing)
	{
		for (int i = 0; i < CachedOpenings.Openings.Num(); ++i)
		{
			FName ActorName = CachedOpenings.Openings[i].Mesh.GetFName();
			
			if (ActorName.IsEqual(FName(*GizmoIdentifier)))
			{
				CachedOpenings.Openings[i].Transform = CurrentTransform;
				CachedOpenings.Openings[i].Mesh.Get()->SetActorTransform(CurrentTransform);
			}

			for (int j = 0; j < LastSpawnedActors.Num(); ++j)
			{
				if (LastSpawnedActors[j]->GetFName().IsEqual(FName(*GizmoIdentifier)))
				{
					LastSpawnedActors[j]->SetActorTransform(CurrentTransform);
				}
			}
		}

		OutputActor->UpdatedGeneratedOpenings(CachedOpenings.Openings);
	}
	else
	{
		for (int i = 0; i < EditedOpenings.Openings.Num(); ++i)
		{
			FName ActorName = EditedOpenings.Openings[i].Mesh.GetFName();
			
			if (ActorName.IsEqual(FName(*GizmoIdentifier)))
			{
				EditedOpenings.Openings[i].Transform = CurrentTransform;
				EditedOpenings.Openings[i].Mesh.Get()->SetActorTransform(CurrentTransform);
			}

			for (int j = 0; j < LastSpawnedActors.Num(); ++j)
			{
				if (LastSpawnedActors[j]->GetFName().IsEqual(FName(*GizmoIdentifier)))
				{
					LastSpawnedActors[j]->SetActorTransform(CurrentTransform);
				}
			}
		}

		OutputActor->UpdatedGeneratedOpenings(EditedOpenings.Openings);
	}
}

void UBuildingGeneratorTool::OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform)
{
	Super::OnGizmoTransformChanged_Handler(GizmoIdentifier, NewTransform);

	// TODO : This doesn't work currently it crashes the engine.
	if (Settings)
	{
		/*const FVector& Loc = NewTransform.GetLocation();
		const FVector& Offset = FVector::UpVector * 100.0f;
	
		FHitResult Hit;
	
		if (GetToolWorld()->LineTraceSingleByChannel(Hit, Loc + Offset, Loc - Offset, ECC_Visibility))
		{
			if (Hit.GetActor() && Hit.GetActor()->IsA(APCG_BuildingGenerator::StaticClass()))
			{
				SetGizmoTransform(GizmoIdentifier, FTransform(NewTransform.GetRotation(), Hit.Location, NewTransform.GetScale3D()), false);
			}
		}
		*/

		UpdateOpeningTransforms(GizmoIdentifier, NewTransform);
		OutputActor->RerunConstructionScripts();
		
	}
	
}


void UBuildingGeneratorTool::OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform, EScriptableToolGizmoStateChangeType ChangeType)
{
	Super::OnGizmoTransformStateChange_Handler(GizmoIdentifier, CurrentTransform, ChangeType);

	if (ChangeType == EScriptableToolGizmoStateChangeType::BeginTransform)
	{
		if (IsCtrlDown() && !bIsCopying)
		{
			if (!bIsEditing)
			{
				// Spawn another actor in the current location of the gizmo
				for (const auto& Opening : CachedOpenings.Openings)
				{
					if(!Opening.Mesh.GetFName().IsEqual(FName(*GizmoIdentifier))) continue;
					SpawnOpeningFromReference(Opening, false);
					//HideAllGizmos();
					bIsCopying = true;
					return;
				}
			}
			else
			{
				// Spawn another actor in the current location of the gizmo
				for (const auto& Opening : EditedOpenings.Openings)
				{
					if(!Opening.Mesh.GetFName().IsEqual(FName(*GizmoIdentifier))) continue;
					SpawnOpeningFromReference(Opening, false);
					//HideAllGizmos();
					bIsCopying = true;
					return;
				}
			}
		}
	}

	if (ChangeType == EScriptableToolGizmoStateChangeType::EndTransform || ChangeType == EScriptableToolGizmoStateChangeType::UndoRedo)
	{
	
		UpdateOpeningTransforms(GizmoIdentifier, CurrentTransform);
		bIsCopying = false;
		OutputActor->RerunConstructionScripts();
	}
	


}

bool UBuildingGeneratorTool::Trace(FHitResult& OutHit, const FInputDeviceRay& DevicePos)
{
	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;

	bool bBeenHit = GetWorld()->LineTraceSingleByChannel(
		OutHit, 
		DevicePos.WorldRay.Origin, 
		DevicePos.WorldRay.Origin + DevicePos.WorldRay.Direction * WORLD_MAX, 
		ECollisionChannel::ECC_Visibility, Params);

	if (bBeenHit)
	{
		BrushLastPosition = BrushPosition;
		BrushPosition = OutHit.ImpactPoint;
		BrushDirection = BrushLastPosition - BrushPosition;
		BrushNormal = OutHit.ImpactNormal;
	}

	const bool bHitTargetActor = OutHit.GetActor() && OutputActor && OutHit.GetActor() == OutputActor;

	return bBeenHit && bHitTargetActor;
}

bool UBuildingGeneratorTool::Trace(TArray<FHitResult>& OutHit, const FInputDeviceRay& DevicePos)
{
	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;

	bool bBeenHit = GetWorld()->SweepMultiByChannel(
		OutHit, 
		DevicePos.WorldRay.Origin, 
		DevicePos.WorldRay.Origin + DevicePos.WorldRay.Direction * WORLD_MAX,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeSphere(25.f),
		Params);

	if (bBeenHit)
	{
		BrushLastPosition = BrushPosition;
		BrushPosition = OutHit[0].ImpactPoint;
		BrushDirection = BrushLastPosition - BrushPosition;
		BrushNormal = OutHit[0].ImpactNormal;
	}

	bool bHitTargetActor = false;
	for (const auto& Hit : OutHit)
	{
		if (Hit.GetActor() && OutputActor && Hit.GetActor() == OutputActor)
		{
			bHitTargetActor = true;
			break;
		}
	}

	return bBeenHit && bHitTargetActor;
}


void UBuildingGeneratorTool::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);
	
	DrawDebugLine(GetWorld(), BrushPosition, (BrushPosition + BrushDirection), BrushColor.ToFColor(false), false, -1, 0, 5);

	if(!OutputActor) return;
	
	if (OutputActor->IsSelected())
	{
		GEditor->SelectActor(OutputActor, false, true);
	}
	
}

void UBuildingGeneratorTool::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);

	switch (ShutdownType)
	{
	case EToolShutdownType::Completed:
		break;
	case EToolShutdownType::Accept:
		HandleAccept();
		break;
	case EToolShutdownType::Cancel:
		HandleCancel();
		break;
	}
	
	Brush->UnregisterComponent();
	Brush->SetStaticMesh(nullptr);
}

void UBuildingGeneratorTool::UpdateAcceptWarnings(EAcceptWarning Warning)
{
	Super::UpdateAcceptWarnings(Warning);
}

bool UBuildingGeneratorTool::CanAccept() const
{
	return Super::CanAccept();
}

void UBuildingGeneratorTool::HandleAccept()
{
	if (!OutputActor)
	{
		return;
	}
	
	GEditor->SelectActor(OutputActor, true, true);

	// Select all of the actors that were generated by this tool
	if (!bIsEditing)
	{
		for (const auto& Opening : CachedOpenings.Openings)
		{
			if(!Opening.Mesh) continue;
			Opening.Mesh->AttachToActor(OutputActor, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
	else
	{
		for (const auto ActorToDestroy : SpawnedActorsToDestroy)
		{
			ActorToDestroy->Destroy();
		}
		
		OutputActor->UpdatedGeneratedOpenings(EditedOpenings.Openings);
		for (const auto& Opening : EditedOpenings.Openings)
		{
			if(!Opening.Mesh) continue;
			Opening.Mesh->AttachToActor(OutputActor, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
}

void UBuildingGeneratorTool::HandleCancel()
{
	if (!OutputActor)
	{
		return;
	}

	if (!bIsEditing)
	{
		OutputActor->Destroy();
		OutputActor = nullptr;
	}
	else
	{
		for (const auto Spawned : SpawnedActorsToDestroy)
		{
			Spawned->SetIsTemporarilyHiddenInEditor(false);
		}

		for (const auto Actor : EditModeSpawnedActors)
		{
			Actor->Destroy();
		}
		
		for (const auto& Opening : CachedOpenings.Openings)
		{
			if(!Opening.Mesh) continue;
			Opening.Mesh->SetActorTransform(Opening.Transform);
			Opening.Mesh->AttachToActor(OutputActor, FAttachmentTransformRules::KeepWorldTransform);
		}
		
		OutputActor->UpdatedGeneratedOpenings(CachedOpenings.Openings);
	}
	
}

void UBuildingGeneratorTool::OnPreBeginPie(bool InStarted)
{
}


#undef LOCTEXT_NAMESPACE
