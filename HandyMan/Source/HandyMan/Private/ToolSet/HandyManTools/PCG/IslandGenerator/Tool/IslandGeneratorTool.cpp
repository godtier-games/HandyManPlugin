// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandGeneratorTool.h"

#include "ToolSet/HandyManTools/PCG/IslandGenerator/Actors/RuntimeIslandGenerator.h"

#define LOCTEXT_NAMESPACE "UIslandGeneratorTool"

UIslandGeneratorTool::UIslandGeneratorTool()
{
	ToolName = LOCTEXT("ToolName", "Island Generator");;

	ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
}

void UIslandGeneratorTool::Setup()
{
	Super::Setup();
	
	EToolsFrameworkOutcomePins PropertyCreationOutcome;
	Settings = Cast<UIslandGeneratorPropertySet>(AddPropertySetOfType(UIslandGeneratorPropertySet::StaticClass(), "Settings", PropertyCreationOutcome));



	Settings->WatchProperty(Settings->bShouldGenerateOnConstruction, [this](bool)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetShouldGenerateOnConstruction(Settings->bShouldGenerateOnConstruction);
		}
	});
	
	Settings->WatchProperty(Settings->RandomSeed, [this](int32)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetRandomSeed(Settings->RandomSeed);
			
		}
	});
	
	Settings->WatchProperty(Settings->IslandChunks, [this](int32)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetIslandChunkCount(Settings->IslandChunks);
		}
	});
	Settings->WatchProperty(Settings->IslandBounds, [this](FVector2D)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetIslandBounds(Settings->IslandBounds);
		}
	});
	Settings->WatchProperty(Settings->MaxSpawnArea, [this](float)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetMaxSpawnArea(Settings->MaxSpawnArea);
		}
	});
	
	Settings->WatchProperty(Settings->IslandDepth, [this](float)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetIslandDepth(Settings->IslandDepth);
		}
	});
	
	Settings->WatchProperty(Settings->bShouldFlattenIsland, [this](bool)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetShouldFlattenIsland(Settings->bShouldFlattenIsland);
		}
	});
	
	Settings->WatchProperty(Settings->MaterialParameterCollection, [this](TObjectPtr<UMaterialParameterCollection>)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetMaterialParameterCollection(Settings->MaterialParameterCollection);
		}
	});
	
	Settings->WatchProperty(Settings->GrassColor, [this](FLinearColor)
	{
		if (IsValid(OutputActor))
		{
			OutputActor->SetGrassColor(Settings->GrassColor);
		}
	});
	
	
	Settings->SilentUpdateWatched();
	
	SpawnOutputActorInstance(Settings);
}

void UIslandGeneratorTool::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);

	switch (ShutdownType) {
	case EToolShutdownType::Completed:
		break;
	case EToolShutdownType::Accept:
		if (OutputActor)
		{
			OutputActor->GetDynamicMeshComponent()->SetComplexAsSimpleCollisionEnabled(true, true);
			OutputActor->GetDynamicMeshComponent()->bEnableComplexCollision = true;

			GEditor->SelectActor(OutputActor, true, true);
		}
		break;
	case EToolShutdownType::Cancel:
		if (OutputActor)
		{
			OutputActor->Destroy();
		}
		break;
	}
}

void UIslandGeneratorTool::OnGizmoTransformStateChange_Handler(FString GizmoIdentifier, FTransform CurrentTransform, EScriptableToolGizmoStateChangeType ChangeType)
{
	if (ChangeType == EScriptableToolGizmoStateChangeType::EndTransform || ChangeType == EScriptableToolGizmoStateChangeType::UndoRedo)
	{
		if (OutputActor)
		{
			OutputActor->SetActorTransform(CurrentTransform);
		}
	}
	
}

void UIslandGeneratorTool::OnGizmoTransformChanged_Handler(FString GizmoIdentifier, FTransform NewTransform)
{
	if (OutputActor)
	{
		OutputActor->SetActorTransform(NewTransform);
	}
}

void UIslandGeneratorTool::SpawnOutputActorInstance(const UIslandGeneratorPropertySet* InSettings, const FTransform& SpawnTransform)
{

	if (GetHandyManAPI() && InSettings)
	{

		// Generate the splines from the input actor
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags = RF_Transactional;
		SpawnInfo.Name = FName("SplineActor");

		auto World = GetToolWorld();
		auto ClassToSpawn = GetHandyManAPI()->GetPCGActorClass(FName(ToolName.ToString()));
		if (auto SpawnedActor =  World->SpawnActor<ARuntimeIslandGenerator>(ClassToSpawn))
		{
			// Initalize the actor
			SpawnedActor->SetActorTransform(SpawnTransform);

			FScriptableToolGizmoOptions GizmoOptions;
			GizmoOptions.GizmoMode = EScriptableToolGizmoMode::TranslationOnly;
			GizmoOptions.TranslationParts = static_cast<int32>(EScriptableToolGizmoTranslation::TranslateAxisX | EScriptableToolGizmoTranslation::TranslateAxisY |
				EScriptableToolGizmoTranslation::TranslatePlaneXY);
			EToolsFrameworkOutcomePins Outcome;
			CreateTRSGizmo("Gizmo", SpawnTransform, GizmoOptions, Outcome);
			OutputActor = (SpawnedActor);
		}
	}
	else
	{
		// TODO : Error Dialogue
	}
}


#undef LOCTEXT_NAMESPACE
