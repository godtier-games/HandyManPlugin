// Copyright Epic Games, Inc. All Rights Reserved.

#include "HandyManModule.h"
#include "HandyManEditorModeCommands.h"
#include "HandyManEditorModeStyle.h"
#include "DetailCustomizations/BrushSize/HandyManBrushSizeCustomization.h"
#include "DetailCustomizations/SculptTool/HandyManSculptToolCustomizations.h"
#include "ToolSet/HandyManTools/Core/MorphTargetCreator/Tool/MorphTargetCreator.h"

#define LOCTEXT_NAMESPACE "HandyManModule"

void FHandyManModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FHandyManModule::OnPostEngineInit);
}

void FHandyManModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FHandyManEditorModeCommands::Unregister();


	// Unregister customizations
	FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");
	if (PropertyEditorModule)
	{
		for (FName ClassName : ClassesToUnregisterOnShutdown)
		{
			PropertyEditorModule->UnregisterCustomClassLayout(ClassName);
		}
		for (FName PropertyName : PropertiesToUnregisterOnShutdown)
		{
			PropertyEditorModule->UnregisterCustomPropertyTypeLayout(PropertyName);
		}
	}
	
	FHandyManEditorModeStyle::Shutdown();
}

void FHandyManModule::OnPostEngineInit()
{
	FHandyManEditorModeStyle::Initialize();
	
	// Register details view customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomPropertyTypeLayout("HandyManBrushToolRadius", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FHandyManBrushSizeCustomization::MakeInstance));
	PropertiesToUnregisterOnShutdown.Add(FHandyManBrushToolRadius::StaticStruct()->GetFName());
	
	PropertyModule.RegisterCustomClassLayout("HandyManSculptBrushProperties", FOnGetDetailCustomizationInstance::CreateStatic(&FHandyManSculptBrushPropertiesDetails::MakeInstance));
	ClassesToUnregisterOnShutdown.Add(UHandyManSculptBrushProperties::StaticClass()->GetFName());
	
	PropertyModule.RegisterCustomClassLayout("MorphTargetBrushSculptProperties", FOnGetDetailCustomizationInstance::CreateStatic(&FHandyManVertexBrushSculptPropertiesDetails::MakeInstance));
	ClassesToUnregisterOnShutdown.Add(UMorphTargetBrushSculptProperties::StaticClass()->GetFName());
	
	PropertyModule.RegisterCustomClassLayout("MorphTargetBrushAlphaProperties", FOnGetDetailCustomizationInstance::CreateStatic(&FHandyManVertexBrushAlphaPropertiesDetails::MakeInstance));
	FHandyManEditorModeCommands::Register();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHandyManModule, HandyMan)