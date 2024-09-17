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

		// Spawn all gizmos for the array of FGeneratedOpening
		for (const auto& Opening : OutputActor->GetGeneratedOpenings())
		{
			if(!Opening.Mesh) continue;
			
			FScriptableToolGizmoOptions GizmoOptions;
			GizmoOptions.bAllowNegativeScaling = false;
			CreateTRSGizmo(Opening.Mesh->GetFName().ToString(), Opening.Mesh->GetActorTransform(), GizmoOptions, PropertyCreationOutcome);

			CachedOpenings.Add(Opening);
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
		if (IsValid(Brush))
		{
			if (!Brush->IsRegistered())
			{
				Brush->RegisterComponentWithWorld(GetWorld());
			}

			if (PickedMesh != Brush->GetStaticMesh())
			{
				Brush->SetStaticMesh(PickedMesh);
			}
			
			Brush->SetWorldLocation(Hit.Location);
			Brush->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));

			if (IsShiftDown() && IsCtrlDown())
			{
				Rotation = UKismetMathLibrary::FindLookAtRotation(
					BrushPosition,
					BrushPosition + BrushDirection);
			}
			else if (IsShiftDown())
			{
				Rotation = UKismetMathLibrary::FindLookAtRotation(
					BrushPosition,
					BrushPosition + BrushNormal);
			}

			Brush->SetWorldRotation(Rotation);
		}
		else
		{
			Brush->SetVisibility(false);
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
	FHitResult Hit;
	const bool bWasHit = Trace(Hit, DragPos);

	LatestPosition = Hit.Location;

	auto Rotation = Hit.Normal.Rotation();
	
	const float DistanceBetween = IsValid(LastSelectedActor) ? FVector::Dist(LastSelectedActor->GetActorLocation(), LatestPosition) : FVector::Dist(LastSpawnedPosition, LatestPosition);


	switch (Button)
	{
	case EScriptableToolMouseButton::LeftButton:
		{

			Brush->SetVisibility(true);
			UpdateBrush(DragPos);

			HideAllGizmos();
			// Update the location of the brush since hover won't be called during drag
			
			/*if (!IsAltDown())
			{
				if (IsShiftDown() && !IsCtrlDown())
				{
					// If the user has the shift key down, scale the mesh

				}
				else if (IsShiftDown() && IsCtrlDown())
				{
					// If the user has the ctrl + shift key down, rotate the mesh

				}
				else if (IsCtrlDown() && !IsShiftDown())
				{
					// If the user has the ctrl key down, Enable snapping
				}
			}*/
		}
		
		break;
	case EScriptableToolMouseButton::RightButton:
		break;
	case EScriptableToolMouseButton::MiddleButton:
		{
			// Check if hit actor is a static mesh actor with the same mesh as our painting mesh
			// If it is, delete the actor
			if (Hit.GetActor() && Hit.GetActor()->IsA(AStaticMeshActor::StaticClass())
				&& Cast<AStaticMeshActor>(Hit.GetActor())->GetStaticMeshComponent()->GetStaticMesh()
				&& LastSpawnedActors.Contains(Hit.GetActor()))
			{
				const FName ActorName = Hit.GetActor()->GetFName();
				LastSpawnedActors.RemoveSingle(Hit.GetActor());
				EToolsFrameworkOutcomePins OutCome;
				DestroyTRSGizmo(Hit.GetActor()->GetFName().ToString(), OutCome);
				Hit.GetActor()->Destroy();

				for (int i = 0; i < CachedOpenings.Num(); ++i)
				{
					if(!CachedOpenings[i].Mesh.GetFName().IsEqual(ActorName)) continue;
					CachedOpenings.RemoveAt(i);
					break;
				}

				if (OutputActor)
				{
					OutputActor->UpdatedGeneratedOpenings(CachedOpenings);
				}
				
			}
		}
		break;
	}
}

void UBuildingGeneratorTool::OnDragBegin(const FInputDeviceRay& StartPosition, const FScriptableToolModifierStates& Modifiers, const EScriptableToolMouseButton& Button)
{

	bIsPainting = true;
	
	/*// if right click is pressed spawn an actor into the world and set it as the selected mesh
	if (Modifiers.bShiftDown == EScriptableToolMouseButton::RightButton)
	{
		
	}

	if (MouseButton == EScriptableToolMouseButton::LeftButton)
	{
	}

	if (MouseButton == EScriptableToolMouseButton::MiddleButton)
	{
		bIsDestroying = true;
	}*/
	
	
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

		
		// If our Mesh is a static mesh actor then we need to spawn a static mesh actor else its just an actor and we should spawn it as is.
		UClass* ClassToSpawn = OpeningRef.Mesh.IsA(UStaticMesh::StaticClass()) ? AStaticMeshActor::StaticClass() : OpeningRef.Mesh.GetClass();

		FActorSpawnParameters Params = FActorSpawnParameters();
		FString name = FString::Format(TEXT("Actor_{0}"), { PaintingMesh->GetFName().ToString() });
		FName fname = MakeUniqueObjectName(nullptr, ClassToSpawn, FName(*name));
		Params.Name = fname;
		Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;

		AActor* actor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, Hit.Location, Rotation, Params);
		actor->GetRootComponent()->SetMobility(EComponentMobility::Type::Movable);
	

		actor->SetActorLabel(fname.ToString());
		LastSpawnedActors.Add(actor);
		if (actor->IsA(AStaticMeshActor::StaticClass()))
		{
			Cast<AStaticMeshActor>(actor)->GetStaticMeshComponent()->SetStaticMesh(PaintingMesh);
		}

		FVector Origin; 
		FVector Extent;
		actor->GetActorBounds(false, Origin, Extent);
		FVector Offset = FVector::ZeroVector;
		FVector MeshScale = FVector::OneVector;
		switch (OpeningRef.Fit)
		{
		case EMeshFitStyle::Flush:
			{
				Offset = actor->GetActorForwardVector() * (Extent);
				const float CurrentX = (Extent * actor->GetActorForwardVector() * 2).Size();
				if (Settings->WallThickness > CurrentX)
				{
					MeshScale = FVector(Settings->WallThickness / CurrentX, 1.0f, 1.0f);
				}
			}
			break;
		case EMeshFitStyle::In_Front:
			break;
		case EMeshFitStyle::Centered:
			{
				Offset = actor->GetActorForwardVector() * (Extent);
				const float CurrentX = (Extent * actor->GetActorForwardVector() * 2).Size();
				if (Settings->WallThickness > CurrentX)
				{
					MeshScale = FVector(Settings->WallThickness / CurrentX, 1.0f, 1.0f);
				}
				else
				{
					MeshScale = FVector(1.0f + (OpeningRef.CenteredOffset * .01f), 1.0f, 1.0f);
				}
			}
			break;
		}
		
		const FTransform SpawnTransform = FTransform(Rotation, Hit.Location, FVector::One());
		actor->SetActorTransform(SpawnTransform);
		actor->GetRootComponent()->SetRelativeScale3D(MeshScale);
		//actor->GetRootComponent()->AddLocalOffset(-Offset);

		

		FGeneratedOpening NewOpening;
		NewOpening.Transform = SpawnTransform;
		NewOpening.Mesh = actor;
		NewOpening.bShouldCutHoleInTargetMesh = OpeningRef.bIsSubtractiveBoolean;
		NewOpening.bShouldApplyBoolean = OpeningRef.bShouldApplyBoolean;
		NewOpening.BooleanShape = OpeningRef.BooleanShape;
		NewOpening.Fit = OpeningRef.Fit;
		
		if (OutputActor)
		{
			OutputActor->AddGeneratedOpeningEntry(NewOpening);
		}

		CachedOpenings.Add(NewOpening);

		// Add a gizmo so in edit mode the user can move without needing to use the modifiers.
		EToolsFrameworkOutcomePins OutCome;
		FScriptableToolGizmoOptions GizmoOptions;
		GizmoOptions.bAllowNegativeScaling = false;
		
		CreateTRSGizmo(actor->GetFName().ToString(), actor->GetActorTransform(), GizmoOptions, OutCome);
		SetGizmoVisible(actor->GetFName().ToString(), false);
	}
	}
}

void UBuildingGeneratorTool::HideAllGizmos()
{
	for (const auto& Gizmo : Gizmos)
	{
		Gizmo.Value->SetVisibility(false);
	}
}

FInputRayHit UBuildingGeneratorTool::CanClickFunc_Implementation(const FInputDeviceRay& PressPos, const EScriptableToolMouseButton& Button)
{
	if (!IsShiftDown())
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
				Brush->SetVisibility(true);

				HideAllGizmos();
				
				return UScriptableToolsUtilityLibrary::MakeInputRayHit(Hit.Distance, nullptr);
			}

			
			bool bHasHitAnOpening = false;
			for (const auto& Opening : CachedOpenings)
			{
				if(!Opening.Mesh.GetFName().IsEqual(Hit.GetActor()->GetFName())) continue;
				bHasHitAnOpening = true;
			}

			if (bHasHitAnOpening)
			{
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
			for (const auto& Opening : CachedOpenings)
			{
				if(!Opening.Mesh.GetFName().IsEqual(Hit.GetActor()->GetFName())) continue;
				Identifier = Hit.GetActor()->GetFName().ToString();
			}

			if (Identifier.IsEmpty()) {return;}

			for (const auto& Gizmo : Gizmos)
			{
				if(!Identifier.Equals(Gizmo.Key))
				{
					Gizmo.Value->SetVisibility(false);
				}
				else
				{
					Gizmo.Value->SetVisibility(true);
					// Turn off the brush so the user can see the gizmo
					Brush->SetVisibility(false);
				}
			}
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
		
	}
	
}

void UBuildingGeneratorTool::UpdateOpeningTransforms(const FString& GizmoIdentifier, const FTransform& CurrentTransform)
{

	for (int i = 0; i < CachedOpenings.Num(); ++i)
	{
		FName ActorName = CachedOpenings[i].Mesh.GetFName();
		AActor* Actor = CachedOpenings[i].Mesh;
		const FGeneratedOpening& OpeningRef = CachedOpenings[i];
			
		if (ActorName.IsEqual(FName(*GizmoIdentifier)))
		{
			FVector Origin;
			FVector Extent;
			Actor->GetActorBounds(false, Origin, Extent);
			FVector Offset =/* OpeningRef.bShouldLayFlush ? ((Actor->GetActorForwardVector() * Extent)) :*/ FVector::ZeroVector;

			FTransform FinalTransform = CurrentTransform;
			FinalTransform.SetLocation(FinalTransform.GetLocation() - Offset);
			CachedOpenings[i].Transform = FinalTransform;
			Actor->SetActorTransform(FinalTransform);
		}
	}
}

void UBuildingGeneratorTool::UpdateOpeningTransforms(const AActor* Opening, const FTransform& CurrentTransform)
{
	for (int i = 0; i < LastSpawnedActors.Num(); ++i)
	{
		AActor* Actor = LastSpawnedActors[i];
			
		if (Actor->GetFName().IsEqual(Opening->GetFName()))
		{
			FVector Origin;
			FVector Extent;
			Actor->GetActorBounds(false, Origin, Extent);
				
			FTransform FinalTransform = CurrentTransform;
			FinalTransform.SetLocation(FinalTransform.GetLocation() - ((Actor->GetActorForwardVector() * Extent)));
			Actor->SetActorTransform(FinalTransform);
		}
	}

	for (int i = 0; i < CachedOpenings.Num(); ++i)
	{
		FName ActorName = CachedOpenings[i].Mesh.GetFName();
		AActor* Actor = CachedOpenings[i].Mesh;
			
		if (ActorName.IsEqual(Opening->GetFName()))
		{
			FVector Origin;
			FVector Extent;
			Actor->GetActorBounds(false, Origin, Extent);
				
			FTransform FinalTransform = CurrentTransform;
			FinalTransform.SetLocation(FinalTransform.GetLocation() - ((Actor->GetActorForwardVector() * Extent)));
			CachedOpenings[i].Transform = FinalTransform;
		}
	}
}

void UBuildingGeneratorTool::OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform, EScriptableToolGizmoStateChangeType ChangeType)
{
	Super::OnGizmoTransformStateChange_Handler(GizmoIdentifier, CurrentTransform, ChangeType);

	if (ChangeType == EScriptableToolGizmoStateChangeType::EndTransform || ChangeType == EScriptableToolGizmoStateChangeType::UndoRedo)
	{
	
		UpdateOpeningTransforms(GizmoIdentifier, CurrentTransform);
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


void UBuildingGeneratorTool::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);
	
	DrawDebugLine(GetWorld(), BrushPosition, (BrushPosition + BrushDirection), BrushColor.ToFColor(false), false, -1, 0, 5);
	
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
	if (OutputActor)
	{
		GEditor->SelectActor(OutputActor, true, true);

		// Select all of the actors that were generated by this tool
		for (const auto& Opening : CachedOpenings)
		{
			if(!Opening.Mesh) continue;
			Opening.Mesh->AttachToActor(OutputActor, FAttachmentTransformRules::KeepWorldTransform);
		}

	}

	
	
}

void UBuildingGeneratorTool::HandleCancel()
{
	if (OutputActor && !bIsEditing)
	{
		OutputActor->Destroy();
		OutputActor = nullptr;
	}
}

void UBuildingGeneratorTool::OnPreBeginPie(bool InStarted)
{
}


#undef LOCTEXT_NAMESPACE
