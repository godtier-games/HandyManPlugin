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
				"ToolWidgets",
				"EditorWidgets",
				"DeveloperSettings",
				"PropertyEditor",
				"StatusBar",
				"Projects",

				"Core",
				"PhysicsCore",
				"RenderCore",

				"GeometryCore",
				"InteractiveToolsFramework",
				"EditorInteractiveToolsFramework",
				"ScriptableToolsFramework",
				"EditorScriptableToolsFramework",

				"ModelingComponents",
				"HoudiniEngineEditor",
				"HoudiniEngineRuntime"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine", 
				"HoudiniEngineEditor",
				"HoudiniEngineRuntime"
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
