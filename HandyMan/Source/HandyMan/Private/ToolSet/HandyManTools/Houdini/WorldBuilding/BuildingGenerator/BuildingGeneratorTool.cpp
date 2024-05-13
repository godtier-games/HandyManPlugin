// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/Houdini/WorldBuilding/BuildingGenerator/BuildingGeneratorTool.h"

#include "HandyManSettings.h"
#include "HoudiniPublicAPI.h"
#include "HoudiniPublicAPIAssetWrapper.h"
#include "InteractiveToolManager.h"
#include "Kismet/KismetStringLibrary.h"

UInteractiveTool* UBuildingGeneratorToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	auto NewTool =  NewObject<UBuildingGeneratorTool>(SceneState.ToolManager);
	NewTool->SetTargetWorld(SceneState.World);
	check(NewTool != nullptr);
	return NewTool;
}

void UBuildingGeneratorProperties::PostAction()
{
	if (ParentTool.IsValid())
	{
		ParentTool->RefreshAction();
	}
}

UBaseScriptableToolBuilder* UBuildingGeneratorTool::GetNewCustomToolBuilderInstance(UObject* Outer)
{
	auto Builder =  NewObject<UBuildingGeneratorToolBuilder>(Outer);
	return Builder;
}

void UBuildingGeneratorTool::Setup()
{
	Super::Setup();

	EToolsFrameworkOutcomePins OutcomePin;
	BuildingModules = Cast<UBuildingGeneratorProperties>(AddPropertySetOfType(UBuildingGeneratorProperties::StaticClass(), TEXT("Building Generator Properties"), OutcomePin));
	BuildingModules->Initialize(this);

	/*Modules->WatchProperty(Modules->BuildingModules, [this](const TArray<FHandyManBuildingModule>& InputModules)
	{
		RefreshAction();
	});*/


	if (GetHandyManAPI())
	{
		CurrentInstance = GetHandyManAPI()->GetMutableHoudiniAPI()->InstantiateAsset
		(
			GetHandyManAPI()->GetHoudiniDigitalAsset(EHandyManToolName::BuildingGenerator),
			FTransform::Identity,
			nullptr,
			nullptr,
			false
		);
	}
}

void UBuildingGeneratorTool::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);
	
	UHandyManSettings* Settings = GetMutableDefault<UHandyManSettings>();
	if(UDataTable* BuildingModuleDataTable = Settings->GetBuildingModuleDataTable())
	{
		for (auto Item : BuildingModuleDataTable->GetRowNames())
		{
			BuildingModuleDataTable->RemoveRow(Item);
		}
	}

	if (UDataTable* BuildingMeshesDataTable = Settings->GetBuildingMeshesDataTable())
	{
		for (auto Item : BuildingMeshesDataTable->GetRowNames())
		{
			BuildingMeshesDataTable->RemoveRow(Item);
		}
	}

	
	
}

void UBuildingGeneratorTool::RefreshAction()
{
	if (!CurrentInstance)
	{
		return;
	}

	TMap<FName, FHoudiniParameterTuple> OutputParameters;
	CurrentInstance->GetParameterTuples(OutputParameters);
	UDataTable* BuildingModuleDataTable = nullptr;
	UDataTable* BuildingMeshesDataTable = nullptr;
	if (BuildingModules->BuildingModules.Num() > 0)
	{
		UHandyManSettings* Settings = GetMutableDefault<UHandyManSettings>();
		BuildingModuleDataTable = Settings->GetBuildingModuleDataTable();
		if(BuildingModuleDataTable)
		{
			for (auto Item : BuildingModuleDataTable->GetRowNames())
			{
				BuildingModuleDataTable->RemoveRow(Item);
			}
		}

		BuildingMeshesDataTable = Settings->GetBuildingMeshesDataTable();
		if (BuildingMeshesDataTable)
		{
			for (auto Item : BuildingMeshesDataTable->GetRowNames())
			{
				BuildingMeshesDataTable->RemoveRow(Item);
			}
		}
	}
	

	if (!BuildingModuleDataTable || !BuildingMeshesDataTable)
	{
		return;
		
	}
	// Do something with the modules
	TArray<UObject*> InputObjects;
	TArray<FHandyManFloorModule> FloorModules;
	TArray<FHandyManBuildingMeshComponent> MeshComponents;
	for (int i = 0; i < BuildingModules->BuildingModules.Num(); i++)
	{
		// Add a tag to the target blockout mesh only if we haven't already
		if (!BuildingModules->BuildingModules[i].TargetBlockoutMesh.IsEmpty())
		{
			for (int j = 0; j < BuildingModules->BuildingModules.Num(); j++)
			{
				if (!BuildingModules->BuildingModules[i].BuildingName.IsEmpty() && BuildingModules->BuildingModules[i].TargetBlockoutMesh[j]->Tags.IsEmpty())
				{
					BuildingModules->BuildingModules[i].TargetBlockoutMesh[j]->Tags.Add(FName(BuildingModules->BuildingModules[i].BuildingName));
				}

				InputObjects.Add(BuildingModules->BuildingModules[i].TargetBlockoutMesh[j]);
			}
		}

		FloorModules.Append(BuildingModules->BuildingModules[i].FloorModules);

		CurrentInstance->SetIntParameterValue("styles", i + 1, 0);
		FString GroupParameterName = FString::Printf(TEXT("group%d"), i + 1);
		FString PatternParameterName = FString::Printf(TEXT("pattern%d"), i + 1);
		CurrentInstance->SetStringParameterValue(FName(GroupParameterName), BuildingModules->BuildingModules[i].BuildingName, 0);
		CurrentInstance->SetStringParameterValue(FName(PatternParameterName), ConstructFloorPatternString(BuildingModules->BuildingModules[i].FloorModules), 0);
		FHandyManHoudiniBuildingModule Module;
		AppendMeshes(BuildingModules->BuildingModules[i].FloorModules, MeshComponents);
	}

	ConstructMeshDataTable(MeshComponents, BuildingMeshesDataTable);
	ConstructFloorPatternsDataTable(FloorModules, BuildingModuleDataTable);

	auto MeshesInput = CurrentInstance->CreateEmptyInput(UHoudiniPublicAPIGeoInput::StaticClass());
	MeshesInput->SetInputObjects({BuildingMeshesDataTable});
	CurrentInstance->SetInputParameter(FName("meshes"), MeshesInput);

	auto FloorInput = CurrentInstance->CreateEmptyInput(UHoudiniPublicAPIGeoInput::StaticClass());
	FloorInput->SetInputObjects({BuildingModuleDataTable});
	CurrentInstance->SetInputParameter(FName("floors"), FloorInput);

	auto WorldInput = CurrentInstance->CreateEmptyInput(UHoudiniPublicAPIWorldInput::StaticClass());
	WorldInput->SetInputObjects(InputObjects);
	CurrentInstance->SetInputAtIndex(0, WorldInput);

	CurrentInstance->Rebuild();

	

		
}

FString UBuildingGeneratorTool::ConstructFloorPatternString(const TArray<FHandyManBuildingMeshComponent>& Pattern)
{

	TMap<int32, FString> FinalStringMap;

	for (int i = 0; i < Pattern.Num(); i++)
	{
		if (Pattern[i].Repetitions > 0)
		{
			FString RepetitionString = FString::Printf(TEXT("[%s]*%d"), *Pattern[i].MeshNameTag.GetTagName().ToString(), Pattern[i].Repetitions);
			FinalStringMap.Add(i, RepetitionString);
			continue;
		}
		FString Addition = i != Pattern.Num() - 1 && Pattern.Num() > 1 ? TEXT("-") : TEXT("");
		FString FloorPatternString = FString::Printf(TEXT("%s%s"), *Pattern[i].MeshNameTag.GetTagName().ToString(), *Addition);
		FinalStringMap.Add(i, FloorPatternString);
	}
	
	TArray<FString> FinalString;
	for (auto Item : FinalStringMap)
	{
		FinalString.EmplaceAt(Item.Key, Item.Value);
	}

	FString CompiledString;
	for (auto Item : FinalStringMap)
	{
		CompiledString += Item.Value;
	}

	FString OutString = FString::Printf(TEXT("<%s>"), *CompiledString);

	return UKismetStringLibrary::Replace(OutString, TEXT("."), TEXT("_"));
}

FString UBuildingGeneratorTool::ConstructFloorPatternString(const TArray<FHandyManFloorModule>& Pattern)
{
	TArray<FString> FullPatternString;

	for (int i = 0; i < Pattern.Num(); i++)
	{
		FString Addition = i != Pattern.Num() - 1 && Pattern.Num() > 1 ? TEXT("-") : TEXT("");
		FString FloorPatternString = FString::Printf(TEXT("%s%s"), *Pattern[i].FloorNameTag.GetTagName().ToString(), *Addition);
		FullPatternString.Add(FloorPatternString);
	}
	
	FString FinalString;
	for (auto Item : FullPatternString)
	{
		FinalString += Item;
	}

	

	FString OutString = FString::Printf(TEXT("<%s>"), *FinalString);

	return UKismetStringLibrary::Replace(OutString, TEXT("."), TEXT("_"));
}

void UBuildingGeneratorTool::AppendMeshes(const TArray<FHandyManFloorModule>& Modules, TArray<FHandyManBuildingMeshComponent>& OutMeshes)
{
	for (auto Item : Modules)
	{
		OutMeshes.Append(Item.Pattern);
	}
}

void UBuildingGeneratorTool::ConstructMeshDataTable(const TArray<FHandyManBuildingMeshComponent>& Meshes, UDataTable*& OutDataTable)
{
	for (auto RowName : OutDataTable->GetRowNames())
	{
		OutDataTable->RemoveRow(RowName);
	}
	
	for (auto Item : Meshes)
	{
		FHandyManHoudiniMeshModule MeshModule;
		MeshModule.Mesh = Item.Mesh;
		MeshModule.MeshName = Item.MeshNameTag.GetTagName().ToString();
		MeshModule.MeshWidth = Item.MeshWidth;
		
		OutDataTable->AddRow(Item.MeshNameTag.GetTagName(), MeshModule);
	}
}

void UBuildingGeneratorTool::ConstructFloorPatternsDataTable(const TArray<FHandyManFloorModule>& Modules, UDataTable*& OutDataTable)
{
	for (auto RowName : OutDataTable->GetRowNames())
    {
    	OutDataTable->RemoveRow(RowName);
    }

	for (auto Item : Modules)
	{
		FHandyManHoudiniBuildingModule FloorModule;
		FloorModule.FloorHeight = Item.FloorHeight;
		FloorModule.FloorName = Item.FloorNameTag.GetTagName().ToString();
		FloorModule.Pattern = ConstructFloorPatternString(Item.Pattern);
		
		OutDataTable->AddRow(Item.FloorNameTag.GetTagName(), FloorModule);
	}

	
	
}
