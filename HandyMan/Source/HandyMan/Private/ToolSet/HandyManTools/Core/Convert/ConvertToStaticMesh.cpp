// Fill out your copyright notice in the Description page of Project Settings.


#include "ConvertToStaticMesh.h"

#include "InteractiveToolManager.h"
#include "ModelingObjectsCreationAPI.h"
#include "ToolTargetManager.h"
#include "Algo/Count.h"
#include "ConversionUtils/SceneComponentToDynamicMesh.h"
#include "Physics/ComponentCollisionUtil.h"
#include "Selection/ToolSelectionUtil.h"
#include "TargetInterfaces/MeshTargetInterfaceTypes.h"
#include "ToolTargets/StaticMeshComponentToolTarget.h"

#define LOCTEXT_NAMESPACE "UConvertToStaticMesh"

UInteractiveTool* UConvertToStaticMeshBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UConvertToStaticMesh* Tool = NewObject<UConvertToStaticMesh>(SceneState.ToolManager);
	TArray<TWeakObjectPtr<UPrimitiveComponent>> Inputs;
	auto TryAddSelected = [&Inputs](UActorComponent* Selected)
	{
		if (UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(Selected))
		{
			if (UE::Conversion::CanConvertSceneComponentToDynamicMesh(PrimComponent))
			{
				Inputs.Emplace(PrimComponent);
			}
		}
	};
	if (SceneState.SelectedComponents.Num() > 0)
	{
		for (UActorComponent* Selected : SceneState.SelectedComponents)
		{
			TryAddSelected(Selected);
		}
	}
	else
	{
		for (AActor* SelectedActor : SceneState.SelectedActors)
		{
			for (UActorComponent* Selected : SelectedActor->GetComponents())
			{
				TryAddSelected(Selected);
			}
		}
	}
	
	Tool->InitializeInputs(MoveTemp(Inputs));

#if WITH_EDITOR
	 
	if (UStaticMeshComponentToolTargetFactory* StaticMeshComponentTargetFactory = SceneState.TargetManager->FindFirstFactoryByType<UStaticMeshComponentToolTargetFactory>())
	{
		EMeshLODIdentifier LOD = StaticMeshComponentTargetFactory->GetActiveEditingLOD();
		Tool->SetTargetLOD(LOD);
	}
#endif

	return Tool;
}

bool UConvertToStaticMeshBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	int32 CanConvertCount = 0;
	auto CanConvert = [](UActorComponent* Comp)->bool { return UE::Conversion::CanConvertSceneComponentToDynamicMesh(Cast<USceneComponent>(Comp)); };
	if (SceneState.SelectedComponents.Num() > 0)
	{
		CanConvertCount = static_cast<int32>(Algo::CountIf(SceneState.SelectedComponents, CanConvert));
	}
	else
	{
		CanConvertCount =
			Algo::TransformAccumulate(SceneState.SelectedActors,
				[&CanConvert](AActor* Actor)
				{
					return static_cast<int>(Algo::CountIf(Actor->GetComponents(), CanConvert));
				},
				0);
	}
	return CanConvertCount > 0;
}


UConvertToStaticMesh::UConvertToStaticMesh()
{
	ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
}


void UConvertToStaticMesh::OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	FHitResult HitResult;
	const bool bHit = Trace(HitResult, ClickPos);
	
	if ( bHit && HitResult.Component.IsValid() && HitResult.GetActor())
	{
		if (!Inputs.Contains(HitResult.Component))
		{
			Inputs.Emplace(HitResult.Component);
			GEditor->SelectActor(HitResult.GetActor(), true, true);
		}
		else
		{
			Inputs.RemoveSwap(HitResult.Component);
			GEditor->SelectActor(HitResult.GetActor(), false, true);
		}
		
	}
}

UBaseScriptableToolBuilder* UConvertToStaticMesh::GetNewCustomToolBuilderInstance(UObject* Outer)
{
	return Cast<UBaseScriptableToolBuilder>(NewObject<UConvertToStaticMeshBuilder>(Outer));
}

void UConvertToStaticMesh::Setup()
{
	Super::Setup();

	SetToolDisplayName(LOCTEXT("ToolName", "Convert"));
	GetToolManager()->DisplayMessage(
		LOCTEXT("OnStartTool", "Convert Meshes to Static Mesh"),
		EToolMessageLevel::UserNotification);
}

void UConvertToStaticMesh::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);

	if (ShutdownType == EToolShutdownType::Accept)
	{
		GetToolManager()->BeginUndoTransaction(LOCTEXT("UConvertToStaticMesh", "Convert Meshes"));

		TArray<AActor*> NewSelectedActors;
		TSet<AActor*> DeleteActors;
		TArray<FCreateMeshObjectParams> NewMeshObjects;

		// Accumulate info for new mesh objects. Do not immediately create them because then
		// the new Actors will get a unique-name incremented suffix, because the convert-from
		// Actors still exist.
		for (TWeakObjectPtr<UPrimitiveComponent> Input : Inputs)
		{
			if (!Input.IsValid())
			{
				continue;
			}
			UPrimitiveComponent* InputComponent = Input.Get();
			UE::Geometry::FDynamicMesh3 SourceMesh;
			FTransform SourceTransform;
			TArray<UMaterialInterface*> ComponentMaterials;
			TArray<UMaterialInterface*> AssetMaterials;
			FText ErrorMessage;

			UE::Conversion::FToMeshOptions Options;
			Options.LODType = UE::Conversion::EMeshLODType::SourceModel;
			Options.LODIndex = 0;
			Options.bUseClosestLOD = true;
			if ((int32)TargetLOD <= (int32)EMeshLODIdentifier::LOD7)
			{
				Options.LODIndex = (int32)TargetLOD;
			}
			else if (TargetLOD == EMeshLODIdentifier::MaxQuality)
			{
				Options.LODType = UE::Conversion::EMeshLODType::MaxAvailable;
			}
			else if (TargetLOD == EMeshLODIdentifier::HiResSource)
			{
				Options.LODType = UE::Conversion::EMeshLODType::HiResSourceModel;
			}

			bool bSuccess = UE::Conversion::SceneComponentToDynamicMesh(InputComponent, Options, false, SourceMesh, SourceTransform, ErrorMessage, &ComponentMaterials, &AssetMaterials);
			if (!bSuccess)
			{
				UE_LOG(LogGeometry, Warning, TEXT("Convert Tool failed to convert %s: %s"), *InputComponent->GetName(), *ErrorMessage.ToString());
				continue;
			}

			AActor* TargetActor = InputComponent->GetOwner();
			check(TargetActor != nullptr);
			DeleteActors.Add(TargetActor);
			
			FString AssetName = TargetActor->GetActorNameOrLabel();

			FCreateMeshObjectParams NewMeshObjectParams;
			NewMeshObjectParams.TargetWorld = InputComponent->GetWorld();
			NewMeshObjectParams.Transform = (FTransform)SourceTransform;
			NewMeshObjectParams.BaseName = AssetName;
			NewMeshObjectParams.Materials = ComponentMaterials;
			NewMeshObjectParams.AssetMaterials = AssetMaterials;
			NewMeshObjectParams.SetMesh(MoveTemp(SourceMesh));
			
			if (UE::Geometry::ComponentTypeSupportsCollision(InputComponent, UE::Geometry::EComponentCollisionSupportLevel::ReadOnly))
			{
				NewMeshObjectParams.bEnableCollision = true;
				UE::Geometry::FComponentCollisionSettings CollisionSettings = UE::Geometry::GetCollisionSettings(InputComponent);
				NewMeshObjectParams.CollisionMode = (ECollisionTraceFlag)CollisionSettings.CollisionTypeFlag;

				UE::Geometry::FSimpleShapeSet3d ShapeSet;
				if (UE::Geometry::GetCollisionShapes(InputComponent, ShapeSet))
				{
					NewMeshObjectParams.CollisionShapeSet = MoveTemp(ShapeSet);
				}
			}

			NewMeshObjects.Add(MoveTemp(NewMeshObjectParams));
		}

		// delete all the existing Actors we want to get rid of
		for (AActor* DeleteActor : DeleteActors)
		{
			DeleteActor->Destroy();
		}

		// spawn new mesh objects
		for (FCreateMeshObjectParams& NewMeshObjectParams : NewMeshObjects)
		{
			NewMeshObjectParams.TypeHint = ECreateObjectTypeHint::StaticMesh;
			FCreateMeshObjectResult Result = UE::Modeling::CreateMeshObject(GetToolManager(), MoveTemp(NewMeshObjectParams));
			if (Result.IsOK())
			{
				NewSelectedActors.Add(Result.NewActor);
			}
		}

		ToolSelectionUtil::SetNewActorSelection(GetToolManager(), NewSelectedActors);

		for (auto NewActorObject : NewSelectedActors)
		{
			UE::Geometry::FAxisAlignedBox3d UseBox = NewActorObject->GetComponentsBoundingBox();
			FVector3d Point = UseBox.Center();

			Point.Z = UseBox.Min.Z;

			
			FTransform NewTransform;
			UE::Geometry::FFrame3d LocalFrame(Point);
			LocalFrame.Transform(NewActorObject->GetActorTransform());
			NewTransform = LocalFrame.ToFTransform();

			// TODO: Actually gotta take a dynamic mesh transform it and then save it to the static mesh.

			NewActorObject->SetActorTransform(NewTransform);
		}

		GetToolManager()->EndUndoTransaction();
	}
}

void UConvertToStaticMesh::HandleAccept()
{
}

void UConvertToStaticMesh::HandleCancel()
{
}

bool UConvertToStaticMesh::Trace(FHitResult& OutHit, const FInputDeviceRay& DevicePos)
{
	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;

	bool bBeenHit = GetWorld()->LineTraceSingleByChannel(
		OutHit, 
		DevicePos.WorldRay.Origin, 
		DevicePos.WorldRay.Origin + DevicePos.WorldRay.Direction * HALF_WORLD_MAX, 
		ECollisionChannel::ECC_Visibility, Params);
	
	return bBeenHit;
}

#undef LOCTEXT_NAMESPACE