// Fill out your copyright notice in the Description page of Project Settings.


#include "BakeAnimToTexture.h"

#include "AnimToTextureBPLibrary.h"
#include "AssetToolsModule.h"
#include "EditorAssetLibrary.h"
#include "HandyManSettings.h"
#include "IAssetTools.h"
#include "Selection.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Materials/MaterialInstanceConstant.h"
#include "ToolSet/HandyManTools/Core/AnimToTexture/Actor/AnimToTextureProxyActor.h"

UBakeAnimToTexture::UBakeAnimToTexture()
{
	ToolName = NSLOCTEXT("UBakeAnimToTexture", "ToolName", "Anim To Texture");
	ToolCategory = NSLOCTEXT("UBakeAnimToTexture", "ToolCategory", "Skeletal Mesh");
	ToolTooltip = NSLOCTEXT("UBakeAnimToTexture", "ToolTooltip", "Bake an array of anim sequences to a texture.");
	ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
	ToolLongName = NSLOCTEXT("UBakeAnimToTexture", "ToolName", "Anim To Texture");
}

void UBakeAnimToTexture::Setup()
{
	Super::Setup();
	
	EToolsFrameworkOutcomePins PropertyCreationOutcome;
	Settings = Cast<UBakeAnimToTexturePropertySet>(AddPropertySetOfType(UBakeAnimToTexturePropertySet::StaticClass(), "Settings", PropertyCreationOutcome));

	UHandyManSettings* HandyManSettings = GetMutableDefault<UHandyManSettings>();
	if (!IsValid(Settings->Source) && HandyManSettings && !HandyManSettings->GetToolsWithBlockedDialogs().Contains(FName(ToolName.ToString())))
	{
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
			NSLOCTEXT
			(
				"UBakeAnimToTexture",
				"ErrorMessage",
				"Pass this tool a skeletal mesh and an array of animations then hit accept and it will automatically bake them into textures. [Disable this popup by adding it to the list of disable dialogs in the Handy Man project settings.]"
			));
	}


	Settings->WatchProperty(Settings->AnimationsToBake, [this](TArray<TObjectPtr<UAnimSequence>>)
	{
		if (IsValid(Settings->Data))
		{
			Settings->Data->AnimSequences.Empty();

			for (int i = 0; i < Settings->AnimationsToBake.Num(); ++i)
			{
				FAnimToTextureAnimSequenceInfo NewInfo;
				NewInfo.AnimSequence = Settings->AnimationsToBake[i];
				Settings->Data->AnimSequences.EmplaceAt(i, NewInfo);
			}

			Settings->Data->SkeletalMesh = Settings->Source;
		}
	});
	
	Settings->WatchProperty(Settings->Source, [this](TObjectPtr<USkeletalMesh>)
	{
		if (IsValid(Settings->Source))
		{
			Initialize();
		}
	});
	
	Settings->WatchProperty(Settings->bEnforcePowerOfTwo, [this](bool)
	{
		if (IsValid(Settings->Data))
		{
			Settings->Data->bEnforcePowerOfTwo = Settings->bEnforcePowerOfTwo;
		}
	});

	Settings->WatchProperty(Settings->Precision, [this](EAnimToTexturePrecision)
{
	if (IsValid(Settings->Data))
	{
		Settings->Data->Precision = Settings->Precision;
	}
});
	

	
	SpawnOutputActorInstance(Settings, FTransform::Identity);
	
	GEditor->GetSelectedActors()->DeselectAll();
	
}

void UBakeAnimToTexture::Shutdown(EToolShutdownType ShutdownType)
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
			// TODO : finalize the actor and have it automatically start playing animations in the viewport
			OutputActor->Destroy();

			

			if (Settings->Target)
			{
				AnimationToTexture(Settings->Data);

				for (const auto&  Item : Settings->Target->GetStaticMaterials())
				{
					if(!Item.MaterialInterface) continue;

					UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(Item.MaterialInterface);

					if(!MaterialInstance) continue;

					UpdateMaterialInstanceFromDataAsset(Settings->Data, MaterialInstance);
				}
			}
			
		}
		break;
	case EToolShutdownType::Cancel:
		if (OutputActor)
		{
			OutputActor->Destroy();
			// TODO: Find all created assets and destroy them.
			FAssetData AssetData = FAssetData(Settings->Source, false);

			const FString FolderPath = FString::Printf(TEXT("%s/%s"), *AssetData.PackagePath.ToString(), *Settings->FolderName);
			UEditorAssetLibrary::DeleteDirectory(FolderPath);
		}
		break;
	}

	/*for (const auto& Viewport : GUnrealEd->GetAllViewportClients())
	{
		if(!Viewport->IsFocused(GEditor->GetActiveViewport())) continue;
		Viewport->SetViewMode(VMI_Lit);
	}*/
}

bool UBakeAnimToTexture::CanAccept() const
{
	return Settings && Settings->HasDataToBake();
}

void UBakeAnimToTexture::SpawnOutputActorInstance(const UBakeAnimToTexturePropertySet* InSettings, const FTransform& SpawnTransform)
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
		if (auto SpawnedActor =  World->SpawnActor<AAnimToTextureProxyActor>(ClassToSpawn))
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

FInputRayHit UBakeAnimToTexture::TestIfHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	return UScriptableToolsUtilityLibrary::MakeInputRayHit_Miss();
}

void UBakeAnimToTexture::OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
}

void UBakeAnimToTexture::OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform)
{
	Super::OnGizmoTransformChanged_Handler(GizmoIdentifier, NewTransform);

}

void UBakeAnimToTexture::OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform,
	EScriptableToolGizmoStateChangeType ChangeType)
{
	Super::OnGizmoTransformStateChange_Handler(GizmoIdentifier, CurrentTransform, ChangeType);
}

void UBakeAnimToTexture::Initialize()
{
	// TODO : Tell the actor to copy the mesh to a dynamic mesh. Then Create a static mesh and write that to the settings data structure.

	if (!IsValid(Settings->Source))
	{
		return;
	}

	CreateDataAsset();
	CreateStaticMesh();
	CreateTextures();
}

void UBakeAnimToTexture::CreateStaticMesh()
{
	const auto AssetData = FAssetData(Settings->Source, false);
	const FString NewSavePath = FString::Printf(TEXT("%s/%s/SM_%s"), *AssetData.PackagePath.ToString(), *Settings->FolderName, *AssetData.AssetName.ToString());
	if (OutputActor)
	{
		if (UStaticMesh* MeshToBake = OutputActor->MakeStaticMesh(Settings->Source, NewSavePath))
		{
			Settings->Target = MeshToBake;
			if (Settings->Data)
			{
				Settings->Data->StaticMesh = MeshToBake;
			}
		}
	}

}

void UBakeAnimToTexture::CreateDataAsset()
{
	const auto AssetData = FAssetData(Settings->Source, false);
	const FString NewSavePath = FString::Printf(TEXT("%s/%s"), *AssetData.PackagePath.ToString(), *Settings->FolderName);
	const FString AssetName = FString::Printf(TEXT("DA_%s"), *AssetData.AssetName.ToString());

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	UObject* DataAssetObject = AssetTools.CreateAsset(AssetName, NewSavePath, UAnimToTextureDataAsset::StaticClass(), nullptr);

	if (UAnimToTextureDataAsset* DataAsset = Cast<UAnimToTextureDataAsset>(DataAssetObject))
	{
		
		switch (Settings->TextureResolution)
		{
		case EAnimTextureResolution::Size_64:
			DataAsset->MaxHeight = 64;
			DataAsset->MaxWidth = 64;
			break;
		case EAnimTextureResolution::Size_128:
			DataAsset->MaxHeight = 128;
			DataAsset->MaxWidth = 128;
			break;
		case EAnimTextureResolution::Size_256:
			DataAsset->MaxHeight = 256;
			DataAsset->MaxWidth = 256;
			break;
		case EAnimTextureResolution::Size_512:
			DataAsset->MaxHeight = 512;
			DataAsset->MaxWidth = 512;
			break;
		case EAnimTextureResolution::Size_1024:
			DataAsset->MaxHeight = 1024;
			DataAsset->MaxWidth = 1024;
			break;
		case EAnimTextureResolution::Size_2048:
			DataAsset->MaxHeight = 2048;
			DataAsset->MaxWidth = 2048;
			break;
		case EAnimTextureResolution::Size_4096:
			DataAsset->MaxHeight = 4096;
			DataAsset->MaxWidth = 4096;
			break;
		case EAnimTextureResolution::Size_8192:
			DataAsset->MaxHeight = 8192;
			DataAsset->MaxWidth = 8192;
			break;
		}

		DataAsset->SkeletalMesh = Settings->Source;
		DataAsset->Precision = Settings->Precision;
		DataAsset->bEnforcePowerOfTwo = Settings->bEnforcePowerOfTwo;
		DataAsset->Mode = EAnimToTextureMode::Bone;
		
		Settings->Data = DataAsset;


	}
	
}

void UBakeAnimToTexture::CreateTextures()
{
	
	const auto AssetData = FAssetData(Settings->Source, false);

	const FString TextureSubFolder = TEXT("Textures");
	const FString NewSavePath = FString::Printf(TEXT("%s/%s/%s"), *AssetData.PackagePath.ToString(), *Settings->FolderName, *TextureSubFolder);
	const FString BonePosName = FString::Printf(TEXT("VAT_%s_BonePosition"), *AssetData.AssetName.ToString());
	const FString BoneRotName = FString::Printf(TEXT("VAT_%s_BoneRotation"), *AssetData.AssetName.ToString());
	const FString BoneWeightName = FString::Printf(TEXT("VAT_%s_BoneWeight"), *AssetData.AssetName.ToString());
	

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	
	UObject* BonePosObj = AssetTools.CreateAsset(BonePosName, NewSavePath, UTexture2D::StaticClass(), nullptr);
	UObject* BoneRotObj = AssetTools.CreateAsset(BoneRotName, NewSavePath, UTexture2D::StaticClass(), nullptr);
	UObject* BoneWeightObj = AssetTools.CreateAsset(BoneWeightName, NewSavePath, UTexture2D::StaticClass(), nullptr);

	if (UTexture2D* BonePosTexture = Cast<UTexture2D>(BonePosObj))
	{
		BonePosTexture->CompressionSettings = TC_HDR;
		Settings->BonePositionTexture = BonePosTexture;

		if (Settings->Data)
		{
			Settings->Data->BonePositionTexture = BonePosTexture;
		}
	}

	if (UTexture2D* BoneRotTexture = Cast<UTexture2D>(BoneRotObj))
	{
		BoneRotTexture->CompressionSettings = TC_HDR;
		Settings->BoneRotationTexture = BoneRotTexture;
		
		if (Settings->Data)
		{
			Settings->Data->BoneRotationTexture = BoneRotTexture;
		}
	}

	if (UTexture2D* BoneWeightTexture = Cast<UTexture2D>(BoneWeightObj))
	{
		BoneWeightTexture->CompressionSettings = TC_HDR;
		Settings->BoneWeightTexture = BoneWeightTexture;
		if (Settings->Data)
		{
			Settings->Data->BoneWeightTexture = BoneWeightTexture;
		}
	}

	
}


///-------------------------------------------------------------
/// PROPERTY SET
///
#if WITH_EDITOR

void UBakeAnimToTexturePropertySet::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (!ParentTool.Get() || !ParentTool->IsA(UBakeAnimToTexture::StaticClass())) return;
	auto OutputActor = GetParentTool<UBakeAnimToTexture>()->OutputActor;

	if(!OutputActor) return;
	
}

#endif