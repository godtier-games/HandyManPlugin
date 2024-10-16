﻿// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HandyMan : ModuleRules
{
	public HandyMan(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"AssetRegistry",
				"Slate",
				"SlateCore",
				"Engine",
				"InputCore",
				"UnrealEd",
				"EditorFramework",
				"EditorSubsystem",
				"ToolWidgets",
				"EditorWidgets",
				"DeveloperSettings",
				"PropertyEditor",
				"StatusBar",
				"Projects",
				"Core",
				"LevelEditor",
				"PhysicsCore",
				"RenderCore",
				"GameplayTags",
				"GeometryCore",
				"GeometryAlgorithms",
				"GeometryCollectionEngine",
				"InteractiveToolsFramework",
				"EditorInteractiveToolsFramework",
				"ScriptableToolsFramework",
				"EditorScriptableToolsFramework",
				"InteractiveToolsFramework",
				"ProceduralMeshComponent",
				"PCG",
				"PCGCompute",
				"PCGBiomeCore",
				"PCGExternalDataInterop",
				"PCGWaterInterop",
				"PCGGeometryScriptInterop",
				"ModelingComponents",
				"Landscape",
				"MeshModelingTools",
				"HandyManGeometryScriptUtils",
				"GeometryScriptingEditor",
				"DynamicMesh", 
				"MeshModelingToolsExp",
				"SkeletalMeshDescription", 
				"AnimToTexture",
				"AnimToTextureEditor"

				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine", 
				"AssetRegistry",
				"CoreUObject",
				"Slate",
				"SlateCore",
				"Engine",
				"LevelEditor",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"ContentBrowserData",
				"StatusBar",
				"Projects",
				"TypedElementRuntime",
				"InteractiveToolsFramework",
				"EditorInteractiveToolsFramework",
				"GeometryFramework",
				"GeometryCore",
				"GeometryCollectionEngine",
				"GeometryProcessingInterfaces",
				"DynamicMesh",
				"ModelingComponents",
				"ModelingComponentsEditorOnly",
				"MeshModelingTools",
				"MeshModelingToolsExp",
				"MeshModelingToolsEditorOnly",
				"MeshModelingToolsEditorOnlyExp",
				"MeshLODToolset",
				"ToolWidgets",
				"EditorWidgets",
				"WidgetRegistration",
				"DeveloperSettings",
				"PropertyEditor",
				"ToolMenus",
				"EditorConfig",
				"ToolPresetAsset",
				"ToolPresetEditor",
				"EditorConfig",
				"ModelingToolsEditorMode",
				"PCG",
				"PCGCompute",
				"PCGBiomeCore",
				"PCGExternalDataInterop",
				"PCGWaterInterop",
				"PCGGeometryScriptInterop", 
				"EditorScriptingUtilities", 
				"HandyManGeometryScriptUtils",
				"GeometryScriptingEditor",
				"GeometryScriptingCore",
				"EditorStyle",
				"MeshMergeUtilities",
				"ChaosCore",
				"Chaos",
				"PhysicsCore",
				"GeometryCollectionEngine", 
				"ModelingEditorUI",
				"SkeletalMeshDescription",
				"MeshDescription",
				"StaticMeshDescription",
				"SkeletalMeshDescription",
				"MeshConversion",
				"AnimToTexture", 
				"AnimToTextureEditor",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
