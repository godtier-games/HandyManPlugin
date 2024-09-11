// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingGeneratorTool.h"

#include "ModelingToolTargetUtil.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "ToolSet/HandyManTools/PCG/BuildingGenerator/Actor/PCG_BuildingGenerator.h"

#define LOCTEXT_NAMESPACE "BuildingGeneratorTool"

UBuildingGeneratorTool::UBuildingGeneratorTool()
{
	ToolName = LOCTEXT("BuildingGeneratorToolName", "Building Generator");
	ToolTooltip = LOCTEXT("BuildingGeneratorToolDescription", "Generate buildings from an input mesh");
	ToolLongName = LOCTEXT("BuildingGeneratorToolLongName", "Generate Building From Blockout");
	ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
	bWantMouseHover = true;
	bUpdateModifiersDuringDrag = true;
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

	GEngine->OnLevelActorAdded().AddUObject(this, &ThisClass::OnLevelActorsAdded);
	GEngine->OnLevelActorDeleted().AddUObject(this, &ThisClass::OnLevelActorsDeleted);
	FEditorDelegates::PreBeginPIE.AddUObject(this, &ThisClass::OnPreBeginPie);
	
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
	
	Settings->WatchProperty(Settings->DesiredFloorHeight, [this](float)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetDesiredFloorHeight(Settings->DesiredFloorHeight);
		}
	});

	Settings->WatchProperty(Settings->WallThickness, [this](float)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetWallThickness(Settings->WallThickness);
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

	SpawnOutputActorInstance(Settings, TargetActor->GetActorTransform());

	
}

bool UBuildingGeneratorTool::OnUpdateHover(const FInputDeviceRay& DevicePos)
{
	FHitResult Hit;
	const bool bWasHit = Trace(Hit, DevicePos);

	auto Rotation = Hit.Normal.Rotation();
	auto PickedMesh = PaintingMesh;
	
	if (IsValid(PickedMesh))
	{
		if (IsValid(Brush))
		{
			if (!Brush->IsRegistered())
			{
				Brush->RegisterComponentWithWorld(GetWorld());
			}

			Brush->SetStaticMesh(PickedMesh);
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

bool UBuildingGeneratorTool::Trace(FHitResult& OutHit, const FInputDeviceRay& DevicePos)
{
	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;

	bool bBeenHit = GetWorld()->LineTraceSingleByChannel(
		OutHit, 
		DevicePos.WorldRay.Origin, 
		DevicePos.WorldRay.Origin + DevicePos.WorldRay.Direction * HALF_WORLD_MAX, 
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

void UBuildingGeneratorTool::OnClickDrag(const FInputDeviceRay& DragPos)
{
	Super::OnClickDrag(DragPos);
	
	FHitResult Hit;
	const bool bWasHit = Trace(Hit, DragPos);

	LatestPosition = Hit.Location;

	auto Rotation = Hit.Normal.Rotation();
	
	const bool bIsPlacingMeshes = bWasHit && IsValid(LastSelectedActor) && bIsPainting;
	const bool bIsEditingMeshes = bWasHit && IsValid(LastSelectedActor) && bIsEditing;

	const float DistanceBetween = IsValid(LastSelectedActor) ? FVector::Dist(LastSelectedActor->GetActorLocation(), LatestPosition) : FVector::Dist(LastSpawnedPosition, LatestPosition);

	
	
	if (bIsPlacingMeshes)
	{
		// If the user has the shift key down, toggle aligning the mesh to the surface
		// If the user has the ctrl key down, delete the mesh
	}
	else if (bIsEditingMeshes)
	{
		// If the user has the shift key down, scale the mesh
		// If the user has the ctrl + shift key down, rotate the mesh
		// If the user has the ctrl key down, Enable snapping
	}
	else if (bIsDestroying)
	{
		// Check if hit actor is a static mesh actor with the same mesh as our painting mesh
		// If it is, delete the actor
		if (Hit.GetActor() && Hit.GetActor()->IsA(AStaticMeshActor::StaticClass())
			&& Cast<AStaticMeshActor>(Hit.GetActor())->GetStaticMeshComponent()->GetStaticMesh()
			&& LastSpawnedActors.Contains(Hit.GetActor()))
		{
			LastSpawnedActors.RemoveSingle(Hit.GetActor());
			Hit.GetActor()->Destroy();
		}
	}
}

void UBuildingGeneratorTool::OnDragBegin_Implementation(FInputDeviceRay StartPosition, const FScriptableToolModifierStates& Modifiers)
{

	if (!PaintingMesh)
	{
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
			LOCTEXT("UBuildingGeneratorTool", "You are trying to add meshes to the building but no mesh is selected. Please select a mesh by dragging it into the scene from the asset or content browser."));
		return;
	}
		
	FHitResult Hit;
	const bool bWasHit = Trace(Hit, StartPosition);
		
	if (bWasHit && IsValid(PaintingMesh))
	{
		bIsPainting = true;

		LastSpawnedPosition = Hit.Location;
		auto Rotation = Hit.Normal.Rotation();

		FActorSpawnParameters Params = FActorSpawnParameters();
		FString name = FString::Format(TEXT("Actor_{0}"), { PaintingMesh->GetFName().ToString() });
		FName fname = MakeUniqueObjectName(nullptr, AStaticMeshActor::StaticClass(), FName(*name));
		Params.Name = fname;
		Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;

		AStaticMeshActor* actor = GetWorld()->SpawnActor<AStaticMeshActor>(Hit.Location, Rotation, Params);

		actor->SetActorLabel(fname.ToString());
		LastSpawnedActors.Add(actor);
		actor->GetStaticMeshComponent()->SetStaticMesh(PaintingMesh);
	}
	
	/*// if right click is pressed spawn an actor into the world and set it as the selected mesh
	if (Modifiers.bShiftDown == EScriptableToolMouseButton::RightButton)
	{
		
	}

	if (MouseButton == EScriptableToolMouseButton::LeftButton)
	{
		bIsEditing = true;
	}

	if (MouseButton == EScriptableToolMouseButton::MiddleButton)
	{
		bIsDestroying = true;
	}*/
	
	
}

void UBuildingGeneratorTool::OnDragEnd_Implementation(FInputDeviceRay EndPosition, const FScriptableToolModifierStates& Modifiers)
{
	bIsDestroying = false;
	bIsPainting = false;
	bIsEditing = false;
}

void UBuildingGeneratorTool::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);
	
	DrawDebugLine(GetWorld(), BrushPosition, (BrushPosition + BrushDirection), BrushColor.ToFColor(false), false, -1, 0, 5);
	
}

void UBuildingGeneratorTool::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);
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
}

void UBuildingGeneratorTool::HandleCancel()
{
}

void UBuildingGeneratorTool::OnPreBeginPie(bool InStarted)
{
}


#undef LOCTEXT_NAMESPACE
