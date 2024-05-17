// Copyright Epic Games, Inc. All Rights Reserved.

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
				"PhysicsCore",
				"RenderCore",
				"GameplayTags",
				"GeometryCore",
				"GeometryAlgorithms",
				"GeometryScriptingCore",
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
				"HoudiniEngineEditor",
				"HoudiniEngineRuntime",
				"Landscape",
				"MeshModelingTools"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine", 
				"HoudiniEngineEditor",
				"HoudiniEngineRuntime",
				"CoreUObject",
				"Slate",
				"SlateCore",
				"Engine",
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
