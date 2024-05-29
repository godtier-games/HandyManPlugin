// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "HandyManSettings.generated.h"

class UPCGAssetWrapper;
class UHoudiniAssetWrapper;
class UHoudiniAsset;
struct FCollectionReference;



UENUM()
enum class EHandyManDefaultMeshObjectType
{
	/** Generate a new Static Mesh Asset (using Generated Asset settings below) and AStaticMeshActor */
	StaticMeshAsset,
	/** Generate a new AVolume Actor */
	VolumeActor,
	/** Generate a new ADynamicMeshActor (stored locally in the Level) */
	DynamicMeshActor
};



UENUM()
enum class EHandyManAssetGenerationBehavior
{
	/** Generate and automatically Save new Assets on creation */
	AutoGenerateAndAutosave,

	/** Generate new Assets and mark as Modified but do not automatically Save */
	AutoGenerateButDoNotAutosave,

	/** Prompt to Save each new Asset upon Creation */
	InteractivePromptToSave
};



UENUM()
enum class EHandyManAssetGenerationLocation
{
	/** All generated assets will be stored in an AutoGenerated folder that is located relative to the World they are being saved in */
	AutoGeneratedWorldRelativeAssetPath,

	/** All generated assets will be stored in a root-level AutoGenerated folder */
	AutoGeneratedGlobalAssetPath,

	/** Generated assets will be stored in the currently-visible Asset Browser folder if available, otherwise at the Auto Generated Asset Path */
	CurrentAssetBrowserPathIfAvailable
};

UENUM()
enum class EHandyManToolName
{
	None UMETA(Hidden),
	IvyTool UMETA(DisplayName = "Ivy Tool"),
	DrapeTool UMETA(DisplayName = "Drape Tool"),
	FenceTool_Smooth UMETA(DisplayName = "Smooth Fence Tool"),
	FenceTool_Easy UMETA(DisplayName = "Easy Fence Tool"),
	BuildingGenerator UMETA(DisplayName = "Building Generator Tool"),
	ScatterGeometryTool UMETA(DisplayName = "Scatter Tool"),
};


/**
 * 
 */
UCLASS(config=Editor)
class HANDYMAN_API UHandyManSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UHandyManSettings();
	// UDeveloperSettings overrides

	virtual FName GetContainerName() const override { return FName("Project"); }
	virtual FName GetCategoryName() const override { return FName("Plugins"); }
	virtual FName GetSectionName() const override { return FName("ModelingMode"); }

	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;

	UHoudiniAssetWrapper* GetDigitalAssetLibrary() const;
	UPCGAssetWrapper* GetPCGActorLibrary() const;
	TArray<EHandyManToolName> GetToolsWithBlockedDialogs() const { return BlockedDialogsArray; }

	UDataTable* GetBuildingModuleDataTable();
	UDataTable* GetBuildingMeshesDataTable();

	FString GetPCGActorLibraryPath() const { return PCGActorLibrary.ToString(); }
	FString GetDigitalAssetLibraryPath() const { return DigitalAssetLibrary.ToString(); }

protected:

	// Global HDA Library asset to use.
	//UPROPERTY(EditAnywhere, config, Category = "Houdini")
	FSoftObjectPath BuildingModuleDataTable = FSoftObjectPath(TEXT("/HandyMan/Data/DT_BuildingModule.DT_BuildingModule"));

	//UPROPERTY(EditAnywhere, config, Category = "Houdini")
	FSoftObjectPath BuildingMeshesDataTable = FSoftObjectPath(TEXT("/HandyMan/Data/DT_BuildingMeshes.DT_BuildingMeshes"));
	
	// Global HDA Library asset to use.
	//UPROPERTY(EditAnywhere, config, Category = "Houdini")
	FSoftObjectPath DigitalAssetLibrary = FSoftObjectPath(TEXT("/HandyMan/Data/HDA_Library.HDA_Library"));

	// Global HDA Library asset to use.
	//UPROPERTY(EditAnywhere, config, Category = "Houdini")
	FSoftObjectPath PCGActorLibrary = FSoftObjectPath(TEXT("/Script/HandyMan.PCGAssetWrapper'/HandyMan/Data/PCG_Library.PCG_Library'"));

	

	/** List of Tool Names that should be blocked from showing dialog windows */
	UPROPERTY(EditAnywhere, config, Category = "Pop Ups")
	TArray<EHandyManToolName> BlockedDialogsArray;
	
	/** Enable/Disable the options to emit Dynamic Mesh Actors in Modeling Mode Tools */
	UPROPERTY()
	bool bEnableDynamicMeshActors = false;

	/** Where should Assets auto-generated by Modeling Tools be stored by default */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|Generated Assets")
	EHandyManAssetGenerationLocation AssetGenerationLocation = EHandyManAssetGenerationLocation::AutoGeneratedWorldRelativeAssetPath;

	/** How should Assets auto-generated by Modeling Tools be handled in terms of saving, naming, etc */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|Generated Assets")
	EHandyManAssetGenerationBehavior AssetGenerationMode = EHandyManAssetGenerationBehavior::AutoGenerateButDoNotAutosave;

public:

	EHandyManAssetGenerationLocation GetAssetGenerationLocation() const
	{
		return bRestrictiveMode
				   ? EHandyManAssetGenerationLocation::AutoGeneratedGlobalAssetPath
				   : AssetGenerationLocation;
	}

	void SetAssetGenerationLocation(const EHandyManAssetGenerationLocation Location)
	{
		AssetGenerationLocation = Location;
	}

	EHandyManAssetGenerationBehavior GetAssetGenerationMode() const
	{
		return bRestrictiveMode
					? EHandyManAssetGenerationBehavior::AutoGenerateButDoNotAutosave
					: AssetGenerationMode;
	}

	void SetAssetGenerationMode(const EHandyManAssetGenerationBehavior Mode)
	{
		AssetGenerationMode = Mode;
	}

	/** What type of Mesh Object should Output Type Setting default to in Modeling Mode Tools (takes effect after Editor restart) */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man")
	EHandyManDefaultMeshObjectType DefaultMeshObjectType = EHandyManDefaultMeshObjectType::StaticMeshAsset;

	/** Assets auto-generated by Modeling Tools are stored at this path, relative to the parent path defined by the Location. Set to an empty string to disable. */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|Generated Assets")
	FString AutoGeneratedAssetPath = TEXT("_INSTANCE");

	/** If true, Auto-Generated Assets created in an unsaved Level will be stored relative to top-level folder, otherwise they will be stored in /Temp and cannot be saved until they are explicitly moved to a permanent location */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|Generated Assets")
	bool bStoreUnsavedLevelAssetsInTopLevelGameFolder = true;

	/** If true, Autogenerated Assets are stored in per-user folders below the Autogen path */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|Generated Assets")
	bool bUsePerUserAutogenSubfolder = true;

	/** Overrides the user name used for per-user folders below the Autogen path. This might be necessary to resolve issues with source control, for example. Note that the per-user folder name might not contain the name exactly as provided. */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|Generated Assets")
	FString AutogenSubfolderUserNameOverride;

	/** If true, Autogenerated Assets have a short random string generated and appended to their name */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|Generated Assets")
	bool bAppendRandomStringToName = true;

	//
	// User Interface
	//

	/** If true, the standard UE Editor Gizmo Mode (ie selected via the Level Editor Viewport toggle) will be used to configure the Modeling Gizmo, otherwise a Combined Gizmo will always be used. It may be necessary to exit and re-enter Modeling Mode after changing this setting. */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|User Interface")
	bool bRespectLevelEditorGizmoMode = false;

	//
	// Selection
	//

	// old preference for mesh selection system that will be disabled in 5.3
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|Selection", meta = (DisplayName="Enable Mesh Selection UI"))
	bool bEnablePersistentSelections = false;

	// new default-enabled preference for the mesh selection system that we will switch to in 5.3
	/** Enable/Disable new Mesh Selection System (Experimental). */
	//UPROPERTY(config, EditAnywhere, Category = "Handy Man|Selection", meta = (DisplayName="Enable Mesh Selection UI"))
	UPROPERTY()
	bool bEnableMeshSelections = true;

	virtual bool GetMeshSelectionsEnabled() const
	{
		return bEnablePersistentSelections;
		//return bEnableMeshSelections;
	}


	DECLARE_MULTICAST_DELEGATE_TwoParams(UHandyManSettingsModified, UObject*, FProperty*);
	UHandyManSettingsModified OnModified;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		OnModified.Broadcast(this, PropertyChangedEvent.Property);

		Super::PostEditChangeProperty(PropertyChangedEvent);
	}

	//
	// Restrictive Mode
	//

	virtual bool InRestrictiveMode() const
	{
		return bRestrictiveMode;
	}

	virtual void SetRestrictiveMode(bool bEnabled)
	{
		bRestrictiveMode = bEnabled;
	}

	virtual FString GetRestrictiveModeAutoGeneratedAssetPath() const
	{
		return bRestrictiveMode ? RestrictiveModeAutoGeneratedAssetPath : FString();
	}

	virtual bool SetRestrictiveModeAutoGeneratedAssetPath(const FString& AssetPath)
	{
		if (bRestrictiveMode)
		{
			RestrictiveModeAutoGeneratedAssetPath = AssetPath;
			return true;
		}
		return false;
	}

protected:

	/** Assets auto-generated by Modeling Tools are stored at this path when in restrictive mode, relative to the package folder path. */
	UPROPERTY(Config)
	FString RestrictiveModeAutoGeneratedAssetPath = TEXT("Meshes");

	bool bRestrictiveMode = false;




public:
	//
	// Settings that are currently stored during a single session but are not stored in the config file
	// (may promote them to persistent settings in the future)
	//

	/** Toggle Absolute World Grid Position snapping */
	UPROPERTY(Transient)
	bool bEnableAbsoluteWorldSnapping = false;
};




/**
 * Defines a color to be used for a particular Tool Palette Section
 */
USTRUCT()
struct FHandyManCustomSectionColor
{
	GENERATED_BODY()

	/** Name of Section in Modeling Mode Tool Palette */
	UPROPERTY(EditAnywhere, Category = "SectionColor")
	FString SectionName = TEXT("");

	/** Custom Header Color */
	UPROPERTY(EditAnywhere, Category = "SectionColor")
	FLinearColor Color = FLinearColor::Gray;
};


/**
 * Defines a color to be used for a particular Tool Palette Tool
 */
USTRUCT()
struct FHandyManCustomToolColor
{
	GENERATED_BODY()

	/**
	 * Name of Section or Tool in Modeling Mode Tool Palette
	 *
	 * Format:
	 * SectionName        (Specifies a default color for all tools in the section.)
	 * SectionName.ToolName        (Specifies an override color for a specific tool in the given section.)
	 */
	UPROPERTY(EditAnywhere, Category = "ToolColor")
	FString ToolName = TEXT("");

	/** Custom Tool Color */
	UPROPERTY(EditAnywhere, Category = "ToolColor")
	FLinearColor Color = FLinearColor::Gray;
};


/**
 * Defines a Named list/set of content-browser Collection names
 */
USTRUCT()
struct FHandyManAssetCollectionSet
{
	GENERATED_BODY()

	/** Name of the set of collections */
	UPROPERTY(EditAnywhere, Category = "CollectionSet")
	FString Name;

	/** List of Collection names */
	UPROPERTY(EditAnywhere, Category = "CollectionSet")
	TArray<FCollectionReference> Collections;
};


UCLASS(config=Editor)
class HANDYMAN_API UHandyManCustomizationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// UDeveloperSettings overrides

	virtual FName GetContainerName() const { return FName("Editor"); }
	virtual FName GetCategoryName() const { return FName("Plugins"); }
	virtual FName GetSectionName() const { return FName("HandyMan"); }

	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;

public:

	/** Toggle between the Legacy Modeling Mode Palette and the new UI (requires exiting and re-entering the Mode) */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|UI Customization")
	bool bUseLegacyModelingPalette = false;

	/** Add the names of Modeling Mode Tool Palette Sections to have them appear at the top of the Tool Palette, in the order listed below. */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|UI Customization")
	TArray<FString> ToolSectionOrder;

	/** Tool Names listed in the array below will appear in a Favorites section at the top of the Modeling Mode Tool Palette */
	UE_DEPRECATED(5.3, "Handy Man favorites are now set through FEditablePalette or the Mode UI itself")
	TArray<FString> ToolFavorites;
	
	/** Custom Section Header Colors for listed Sections in the Modeling Mode Tool Palette */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|UI Customization")
	TArray<FHandyManCustomSectionColor> SectionColors;

	/**
	 * Custom Tool Colors for listed Tools in the Modeling Mode Tool Palette.
	 * 
	 * Format:
	 * SectionName        (Specifies a default color for all tools in the section.)
	 * SectionName.ToolName        (Specifies an override color for a specific tool in the given section.)
	 */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|UI Customization")
	TArray<FHandyManCustomToolColor> ToolColors;


	/**
	 * A Brush Alpha Set is a named list of Content Browser Collections, which will be shown as a separate tab in 
	 * the Texture Asset Picker used in various Modeling Mode Tools when selecting a Brush Alpha (eg in Sculpting)
	 */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|Tool Assets")
	TArray<FHandyManAssetCollectionSet> BrushAlphaSets;


	/**
	 * If true, the category labels will be shown on the toolbar buttons, else they will be hidden
	 */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|UI Customization")
	bool bShowCategoryButtonLabels = true;
	
	/**
	 * If true, Tool buttons will always be shown when in a Tool. By default they will be hidden.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Handy Man|UI Customization")
	bool bAlwaysShowToolButtons = false;

public:

	// saved-state for various mode settings that are configured via UI toggles/etc, and not exposed in settings dialog

	UPROPERTY(config)
	int32 LastMeshSelectionDragMode = 0;

	UPROPERTY(config)
	int32 LastMeshSelectionLocalFrameMode = 0;

public:

	// saved-state for various mode settings that does not persist between editor runs

	UPROPERTY()
	int32 LastMeshSelectionElementType = 0;

	UPROPERTY()
	int32 LastMeshSelectionTopologyMode = 0;

	UPROPERTY()
	bool bLastMeshSelectionVolumeToggle = true;

	UPROPERTY()
	bool bLastMeshSelectionStaticMeshToggle = true;


};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "Engine/EngineTypes.h"
#endif

