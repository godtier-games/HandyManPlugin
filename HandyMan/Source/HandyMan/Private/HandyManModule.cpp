// Copyright Epic Games, Inc. All Rights Reserved.

#include "HandyManModule.h"
#include "HandyManEditorModeCommands.h"
#include "HandyManEditorModeStyle.h"

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
	FHandyManEditorModeStyle::Shutdown();
}

void FHandyManModule::OnPostEngineInit()
{
	FHandyManEditorModeStyle::Initialize();
	FHandyManEditorModeCommands::Register();

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHandyManModule, HandyManEditorMode)