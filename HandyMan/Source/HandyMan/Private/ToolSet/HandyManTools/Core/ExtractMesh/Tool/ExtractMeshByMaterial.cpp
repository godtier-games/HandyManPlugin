// Fill out your copyright notice in the Description page of Project Settings.


#include "ExtractMeshByMaterial.h"

#include "HandyManSettings.h"
#include "Selection.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "ToolSet/HandyManTools/Core/ExtractMesh/Actor/ExtractMeshProxyActor.h"

UExtractMeshByMaterial::UExtractMeshByMaterial()
{
	ToolName = NSLOCTEXT("UExtractMeshByMaterial", "ToolName", "Skeletal Mesh Extractor");
	ToolCategory = NSLOCTEXT("UExtractMeshByMaterial", "ToolCategory", "Mesh Edit");
	ToolTooltip = NSLOCTEXT("UExtractMeshByMaterial", "ToolTooltip", "Extract and save mesh assets based on material ID's");
	ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
	ToolLongName = NSLOCTEXT("UExtractMeshByMaterial", "ToolName", "Skeletal Mesh Extractor");
}

void UExtractMeshByMaterial::Setup()
{
	Super::Setup();
	
	EToolsFrameworkOutcomePins PropertyCreationOutcome;
	Settings = Cast<UExtractMeshByMaterialPropertySet>(AddPropertySetOfType(UExtractMeshByMaterialPropertySet::StaticClass(), "Settings", PropertyCreationOutcome));

	UHandyManSettings* HandyManSettings = GetMutableDefault<UHandyManSettings>();
	if (!IsValid(Settings->InputMesh) && HandyManSettings && !HandyManSettings->GetToolsWithBlockedDialogs().Contains(FName(ToolName.ToString())))
	{
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
			NSLOCTEXT
			(
				"UBuildingGeneratorTool",
				"ErrorMessage",
				"This tool will automatically initialize the mesh to cut when you have an Input Mesh & you have set a name to save the new asset as. Disable this popup by adding this tool's name to the BlockedDialogsArray in the Handy Man Settings"
			));
	}
	
	Settings->WatchProperty(Settings->MeshDisplayOffset, [this](float)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetMeshOffset(Settings->MeshDisplayOffset);
		}
	});

	Settings->WatchProperty(Settings->bMergeMeshes, [this](bool)
	{
		if (IsValid(OutputActor) && Settings->bMergeMeshes)
		{
			OutputActor->SetMeshOffset(0);
		}
	});

	
	SpawnOutputActorInstance(Settings, FTransform::Identity);
	
	GEditor->GetSelectedActors()->DeselectAll();
	
}

void UExtractMeshByMaterial::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);

	GEditor->GetSelectedActors()->DeselectAll();
	GUnrealEd->edactUnHideAll(GetToolWorld());

	switch (ShutdownType)
	{
	case EToolShutdownType::Completed:
		break;
	case EToolShutdownType::Accept:
		if (OutputActor)
		{
			OutputActor->SaveObjects(Settings->MeshInfo, Settings->FolderName, Settings->MergedAssetName, Settings->bMergeMeshes, Settings->bSaveMeshesAsStaticMesh);
		}
		break;
	case EToolShutdownType::Cancel:
		if (OutputActor)
		{
			OutputActor->Destroy();
		}
		break;
	}

	/*for (const auto& Viewport : GUnrealEd->GetAllViewportClients())
	{
		if(!Viewport->IsFocused(GEditor->GetActiveViewport())) continue;
		Viewport->SetViewMode(VMI_Lit);
	}*/
}

bool UExtractMeshByMaterial::CanAccept() const
{
	return Settings && IsValid(Settings->InputMesh);
}

void UExtractMeshByMaterial::SpawnOutputActorInstance(const UExtractMeshByMaterialPropertySet* InSettings, const FTransform& SpawnTransform)
{
	GEditor->GetSelectedActors()->DeselectAll();
	
	if (GetHandyManAPI() && InSettings)
	{

		// Generate the splines from the input actor
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags = RF_Transactional;
		SpawnInfo.Name = FName("MeshExtractor");

		auto World = GetToolWorld();
		auto ClassToSpawn = GetHandyManAPI()->GetPCGActorClass(FName(ToolName.ToString()));
		if (auto SpawnedActor =  World->SpawnActor<AExtractMeshProxyActor>(ClassToSpawn))
		{
			// Initalize the actor
			SpawnedActor->SetActorTransform(SpawnTransform);
			SpawnedActor->RerunConstructionScripts();
			OutputActor = (SpawnedActor);
			
		}
		
		
		GEditor->SelectActor(OutputActor, true, true);
		
		GEditor->MoveViewportCamerasToActor(*OutputActor, true);
		
		/*for (const auto& Viewport : GUnrealEd->GetAllViewportClients())
		{
			if(!Viewport->IsFocused(GEditor->GetActiveViewport())) continue;
			Viewport->SetViewMode(VMI_Unlit);
		}*/
	}
	else
	{
		// TODO : Error Dialogue
	}
}

FInputRayHit UExtractMeshByMaterial::TestIfHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{

	
	return UScriptableToolsUtilityLibrary::MakeInputRayHit_Miss();
}

void UExtractMeshByMaterial::OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
}

void UExtractMeshByMaterial::OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform)
{
	Super::OnGizmoTransformChanged_Handler(GizmoIdentifier, NewTransform);

}

void UExtractMeshByMaterial::OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform,
	EScriptableToolGizmoStateChangeType ChangeType)
{
	Super::OnGizmoTransformStateChange_Handler(GizmoIdentifier, CurrentTransform, ChangeType);
}


void UExtractMeshByMaterial::SaveDuplicate()
{

	OutputActor->SaveObjects(Settings->MeshInfo, Settings->FolderName);
}


///-------------------------------------------------------------
/// PROPERTY SET
///
#if WITH_EDITOR

void UExtractMeshByMaterialPropertySet::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (!ParentTool.Get() || !ParentTool->IsA(UExtractMeshByMaterial::StaticClass())) return;
	auto* OutputActor = GetParentTool<UExtractMeshByMaterial>()->OutputActor;

	if(!OutputActor) return;

	if (PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, InputMesh))
	{
		if (!InputMesh)
		{
			return;
		}
		
		OutputActor->ExtractMeshInfo(InputMesh, MeshInfo);
		
	}

	if (PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MeshInfo))
	{
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd)
		{
			if (MeshInfo.Num() > OutputActor->GetExtractedMeshInfo().Num())
			{
				MeshInfo.RemoveAt(MeshInfo.Num() - 1);
			}
		}

		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayRemove)
		{
			OutputActor->UpdateExtractedInfo(MeshInfo);
		}
	}
}

#endif