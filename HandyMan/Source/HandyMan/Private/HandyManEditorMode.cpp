// Copyright Epic Games, Inc. All Rights Reserved.

#include "HandyManEditorMode.h"

#include "EditorModelingObjectsCreationAPI.h"
#include "HandyManEditorModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "HandyManEditorModeCommands.h"

#include "EdModeInteractiveToolsContext.h"
#include "Modules/ModuleManager.h"
#include "ILevelEditor.h"
#include "LevelEditor.h"
#include "InteractiveTool.h"
#include "SLevelViewport.h"
#include "Application/ThrottleManager.h"

#include "InteractiveToolManager.h"

#include "BaseGizmos/TransformGizmoUtil.h"
#include "Snapping/ModelingSceneSnappingManager.h"

#include "ScriptableToolBuilder.h"
#include "ScriptableToolSet.h"

#include "InteractiveToolQueryInterfaces.h" // IInteractiveToolExclusiveToolAPI
#include "ModelingModeAssetUtils.h"
#include "ToolContextInterfaces.h"
#include "Components/BrushComponent.h"
#include "Selection/StaticMeshSelector.h"
#include "Selection/VolumeSelector.h"


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 

// step 2: register a ToolBuilder in FHandyManEditorMode::Enter() below


#define LOCTEXT_NAMESPACE "HandyManEditorMode"
const FEditorModeID UHandyManEditorMode::EM_HandyManEditorModeId = TEXT("EM_HandyManEditorMode");


namespace
{
FString GetToolName(const UInteractiveTool& Tool)
{
	const FString* ToolName = FTextInspector::GetSourceString(Tool.GetToolInfo().ToolDisplayName);
	return ToolName ? *ToolName : FString(TEXT("<Invalid ToolName>"));
}
}

UHandyManEditorMode::UHandyManEditorMode()
{
	Info = FEditorModeInfo(
		EM_HandyManEditorModeId,
		LOCTEXT("HandyManEditorModeName", "Handy Man Tools"),
		FSlateIcon("HandyManEditorModeStyle", "LevelEditor.HandyManEditorMode", "LevelEditor.HandyManEditorMode.Small"),
		true,
		999999);
}

UHandyManEditorMode::UHandyManEditorMode(FVTableHelper& Helper)
	: UBaseLegacyWidgetEdMode(Helper)
{
}

UHandyManEditorMode::~UHandyManEditorMode()
{
}

bool UHandyManEditorMode::ProcessEditDelete()
{
	if (UEdMode::ProcessEditDelete())
	{
		return true;
	}

	// for now we disable deleting in an Accept-style tool because it can result in crashes if we are deleting target object
	if ( GetToolManager()->HasAnyActiveTool() && GetToolManager()->GetActiveTool(EToolSide::Mouse)->HasAccept() )
	{
		GetToolManager()->DisplayMessage(
			LOCTEXT("CannotDeleteWarning", "Cannot delete objects while this Tool is active"), EToolMessageLevel::UserWarning);
		return true;
	}

	return false;
}


bool UHandyManEditorMode::ProcessEditCut()
{
	// for now we disable deleting in an Accept-style tool because it can result in crashes if we are deleting target object
	if (GetToolManager()->HasAnyActiveTool() && GetToolManager()->GetActiveTool(EToolSide::Mouse)->HasAccept())
	{
		GetToolManager()->DisplayMessage(
			LOCTEXT("CannotCutWarning", "Cannot cut objects while this Tool is active"), EToolMessageLevel::UserWarning);
		return true;
	}

	return false;
}


void UHandyManEditorMode::ActorSelectionChangeNotify()
{
	// would like to clear selection here, but this is called multiple times, including after a transaction when
	// we cannot identify that the selection should not be cleared
}


bool UHandyManEditorMode::CanAutoSave() const
{
	// prevent autosave if any tool is active
	return GetToolManager()->HasAnyActiveTool() == false;
}

bool UHandyManEditorMode::ShouldDrawWidget() const
{ 
	// hide standard xform gizmo if we have an active tool
	if (GetInteractiveToolsContext() != nullptr && GetToolManager()->HasAnyActiveTool())
	{
		return false;
	}

	return UBaseLegacyWidgetEdMode::ShouldDrawWidget(); 
}

void UHandyManEditorMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	Super::Tick(ViewportClient, DeltaTime);

	if (Toolkit.IsValid())
	{
		FHandyManEditorModeToolkit* ModeToolkit = (FHandyManEditorModeToolkit*)Toolkit.Get();
		ModeToolkit->EnableShowRealtimeWarning(ViewportClient->IsRealtime() == false);
	}
}


void UHandyManEditorMode::Enter()
{
	UEdMode::Enter();

	// listen to post-build
	GetToolManager()->OnToolPostBuild.AddUObject(this, &UHandyManEditorMode::OnToolPostBuild);

	//// forward shutdown requests
	//GetToolManager()->OnToolShutdownRequest.BindLambda([this](UInteractiveToolManager*, UInteractiveTool* Tool, EToolShutdownType ShutdownType)
	//{
	//	GetInteractiveToolsContext()->EndTool(ShutdownType); 
	//	return true;
	//});

	// register gizmo helper
	UE::TransformGizmoUtil::RegisterTransformGizmoContextObject(GetInteractiveToolsContext());

	// register snapping manager
	UE::Geometry::RegisterSceneSnappingManager(GetInteractiveToolsContext());
	//SceneSnappingManager = UE::Geometry::FindModelingSceneSnappingManager(GetToolManager());

	const FHandyManEditorModeCommands& ModeToolCommands = FHandyManEditorModeCommands::Get();

	// enable realtime viewport override
	ConfigureRealTimeViewportsOverride(true);

	// register object creation api
	UEditorModelingObjectsCreationAPI* ModelCreationAPI = UEditorModelingObjectsCreationAPI::Register(GetInteractiveToolsContext());
	if (ModelCreationAPI)
	{
		ModelCreationAPI->GetNewAssetPathNameCallback.BindLambda([](const FString& BaseName, const UWorld* TargetWorld, FString SuggestedFolder)
		{
			return UE::Modeling::GetNewAssetPathName(BaseName, TargetWorld, SuggestedFolder);
		});
		MeshCreatedEventHandle = ModelCreationAPI->OnModelingMeshCreated.AddLambda([this](const FCreateMeshObjectResult& CreatedInfo) 
		{
			if (CreatedInfo.NewAsset != nullptr)
			{
				UE::Modeling::OnNewAssetCreated(CreatedInfo.NewAsset);
				// If we are creating a new asset or component, it should be initially unlocked in the Selection system.
				// Currently have no generic way to do this, the Selection Manager does not necessarily support Static Meshes
				// or Brush Components. So doing it here...
				if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(CreatedInfo.NewAsset))
				{
					FStaticMeshSelector::SetAssetUnlockedOnCreation(StaticMesh);
				}
			}
			if ( UBrushComponent* BrushComponent = Cast<UBrushComponent>(CreatedInfo.NewComponent) )
			{
				FVolumeSelector::SetComponentUnlockedOnCreation(BrushComponent);
			}
		});
		TextureCreatedEventHandle = ModelCreationAPI->OnModelingTextureCreated.AddLambda([](const FCreateTextureObjectResult& CreatedInfo)
		{
			if (CreatedInfo.NewAsset != nullptr)
			{
				UE::Modeling::OnNewAssetCreated(CreatedInfo.NewAsset);
			}
		});
		MaterialCreatedEventHandle = ModelCreationAPI->OnModelingMaterialCreated.AddLambda([](const FCreateMaterialObjectResult& CreatedInfo)
		{
			if (CreatedInfo.NewAsset != nullptr)
			{
				UE::Modeling::OnNewAssetCreated(CreatedInfo.NewAsset);
			}
		});
	}



	ScriptableTools = NewObject<UHandyManScriptableToolSet>(this);
	// find all the Tool Blueprints
	ScriptableTools->ReinitializeCustomScriptableTools();
	// register each of them with ToolManager
	ScriptableTools->ForEachScriptableTool([&](UClass* ToolClass, UInteractiveToolBuilder* ToolBuilder) 
	{
		FString UseName = ToolClass->GetName();
		GetToolManager(EToolsContextScope::EdMode)->RegisterToolType(UseName, ToolBuilder);
	});
	
	// todoz
	GetToolManager()->SelectActiveToolType(EToolSide::Left, TEXT("BeginMeshInspectorTool"));

	BlueprintPreCompileHandle = GEditor->OnBlueprintPreCompile().AddUObject(this, &UHandyManEditorMode::OnBlueprintPreCompile); 
	BlueprintCompiledHandle = GEditor->OnBlueprintCompiled().AddUObject(this, &UHandyManEditorMode::OnBlueprintCompiled); 

	// do any toolkit UI initialization that depends on the mode setup above
	if (Toolkit.IsValid())
	{
		FHandyManEditorModeToolkit* ModeToolkit = (FHandyManEditorModeToolkit*)Toolkit.Get();
		ModeToolkit->InitializeAfterModeSetup();
		ModeToolkit->ForceToolPaletteRebuild();
	}
}


void UHandyManEditorMode::OnBlueprintPreCompile(UBlueprint* Blueprint)
{
	// would be nice to only do this if the Blueprint being compiled is being used for this tool...
	if (GetToolManager()->HasActiveTool(EToolSide::Left))
	{
		GetToolManager()->DeactivateTool(EToolSide::Left, EToolShutdownType::Cancel);
	}
}


void UHandyManEditorMode::OnBlueprintCompiled()
{
	// Probably not necessary to always rebuild the palette here. But currently this lets us respond
	// to changes in the tool name/setting/etc
	if (Toolkit.IsValid())
	{
		FHandyManEditorModeToolkit* ModeToolkit = (FHandyManEditorModeToolkit*)Toolkit.Get();
		ModeToolkit->ForceToolPaletteRebuild();
	}
}

void UHandyManEditorMode::Exit()
{
	GEditor->OnBlueprintPreCompile().Remove(BlueprintPreCompileHandle);

	// exit any exclusive active tools w/ cancel
	if (UInteractiveTool* ActiveTool = GetToolManager()->GetActiveTool(EToolSide::Left))
	{
		if (Cast<IInteractiveToolExclusiveToolAPI>(ActiveTool))
		{
			GetToolManager()->DeactivateTool(EToolSide::Left, EToolShutdownType::Cancel);
		}
	}

	UE::Geometry::DeregisterSceneSnappingManager(GetInteractiveToolsContext());
	UE::TransformGizmoUtil::DeregisterTransformGizmoContextObject(GetInteractiveToolsContext());


	// deregister transform gizmo context object
	UE::TransformGizmoUtil::DeregisterTransformGizmoContextObject(GetInteractiveToolsContext());
	
	// clear realtime viewport override
	ConfigureRealTimeViewportsOverride(false);

	// Call base Exit method to ensure proper cleanup
	UEdMode::Exit();
}

void UHandyManEditorMode::OnToolsContextRender(IToolsContextRenderAPI* RenderAPI)
{
}

bool UHandyManEditorMode::ShouldToolStartBeAllowed(const FString& ToolIdentifier) const
{
	if (UInteractiveToolManager* Manager = GetToolManager())
	{
		if (UInteractiveTool* Tool = Manager->GetActiveTool(EToolSide::Left))
		{
			IInteractiveToolExclusiveToolAPI* ExclusiveAPI = Cast<IInteractiveToolExclusiveToolAPI>(Tool);
			if (ExclusiveAPI)
			{
				return false;
			}
		}
	}
	return Super::ShouldToolStartBeAllowed(ToolIdentifier);
}



void UHandyManEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FHandyManEditorModeToolkit);
}

void UHandyManEditorMode::OnToolPostBuild(
	UInteractiveToolManager* InToolManager, EToolSide InSide, 
	UInteractiveTool* InBuiltTool, UInteractiveToolBuilder* InToolBuilder, const FToolBuilderState& ToolState)
{
}

void UHandyManEditorMode::OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	// disable slate throttling so that Tool background computes responding to sliders can properly be processed
	// on Tool Tick. Otherwise, when a Tool kicks off a background update in a background thread, the computed
	// result will be ignored until the user moves the slider, ie you cannot hold down the mouse and wait to see
	// the result. This apparently broken behavior is currently by-design.
	FSlateThrottleManager::Get().DisableThrottle(true);
}

void UHandyManEditorMode::OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	// re-enable slate throttling (see OnToolStarted)
	FSlateThrottleManager::Get().DisableThrottle(false);
}

void UHandyManEditorMode::BindCommands()
{
	const FHandyManEditorModeCommands& ToolManagerCommands = FHandyManEditorModeCommands::Get();
	const TSharedRef<FUICommandList>& CommandList = Toolkit->GetToolkitCommands();

	CommandList->MapAction(
		ToolManagerCommands.AcceptActiveTool,
		FExecuteAction::CreateLambda([this]() { 
			GetInteractiveToolsContext()->EndTool(EToolShutdownType::Accept); 
		}),
		FCanExecuteAction::CreateLambda([this]() { return GetInteractiveToolsContext()->CanAcceptActiveTool(); }),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateLambda([this]() {return GetInteractiveToolsContext()->ActiveToolHasAccept(); }),
		EUIActionRepeatMode::RepeatDisabled
		);

	CommandList->MapAction(
		ToolManagerCommands.CancelActiveTool,
		FExecuteAction::CreateLambda([this]() { GetInteractiveToolsContext()->EndTool(EToolShutdownType::Cancel); }),
		FCanExecuteAction::CreateLambda([this]() { return GetInteractiveToolsContext()->CanCancelActiveTool(); }),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateLambda([this]() {return GetInteractiveToolsContext()->ActiveToolHasAccept(); }),
		EUIActionRepeatMode::RepeatDisabled
		);

	CommandList->MapAction(
		ToolManagerCommands.CompleteActiveTool,
		FExecuteAction::CreateLambda([this]() { GetInteractiveToolsContext()->EndTool(EToolShutdownType::Completed); }),
		FCanExecuteAction::CreateLambda([this]() { return GetInteractiveToolsContext()->CanCompleteActiveTool(); }),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateLambda([this]() {return GetInteractiveToolsContext()->CanCompleteActiveTool(); }),
		EUIActionRepeatMode::RepeatDisabled
		);

	// These aren't activated by buttons but have default chords that bind the keypresses to the action.
	CommandList->MapAction(
		ToolManagerCommands.AcceptOrCompleteActiveTool,
		FExecuteAction::CreateLambda([this]() { AcceptActiveToolActionOrTool(); }),
		FCanExecuteAction::CreateLambda([this]() {
				return GetInteractiveToolsContext()->CanAcceptActiveTool() || GetInteractiveToolsContext()->CanCompleteActiveTool();
			}),
		FGetActionCheckState(),
		FIsActionButtonVisible(),
		EUIActionRepeatMode::RepeatDisabled);

	CommandList->MapAction(
		ToolManagerCommands.CancelOrCompleteActiveTool,
		FExecuteAction::CreateLambda([this]() { CancelActiveToolActionOrTool(); }),
		FCanExecuteAction::CreateLambda([this]() {
				return GetInteractiveToolsContext()->CanCompleteActiveTool() || GetInteractiveToolsContext()->CanCancelActiveTool();
			}),
		FGetActionCheckState(),
		FIsActionButtonVisible(),
		EUIActionRepeatMode::RepeatDisabled);
}


void UHandyManEditorMode::AcceptActiveToolActionOrTool()
{
	// if we have an active Tool that implements 
	if (GetToolManager()->HasAnyActiveTool())
	{
		UInteractiveTool* Tool = GetToolManager()->GetActiveTool(EToolSide::Mouse);
		IInteractiveToolNestedAcceptCancelAPI* CancelAPI = Cast<IInteractiveToolNestedAcceptCancelAPI>(Tool);
		if (CancelAPI && CancelAPI->SupportsNestedAcceptCommand() && CancelAPI->CanCurrentlyNestedAccept())
		{
			bool bAccepted = CancelAPI->ExecuteNestedAcceptCommand();
			if (bAccepted)
			{
				return;
			}
		}
	}

	const EToolShutdownType ShutdownType = GetInteractiveToolsContext()->CanAcceptActiveTool() ? EToolShutdownType::Accept : EToolShutdownType::Completed;
	GetInteractiveToolsContext()->EndTool(ShutdownType);
}


void UHandyManEditorMode::CancelActiveToolActionOrTool()
{
	// if we have an active Tool that implements 
	if (GetToolManager()->HasAnyActiveTool())
	{
		UInteractiveTool* Tool = GetToolManager()->GetActiveTool(EToolSide::Mouse);
		IInteractiveToolNestedAcceptCancelAPI* CancelAPI = Cast<IInteractiveToolNestedAcceptCancelAPI>(Tool);
		if (CancelAPI && CancelAPI->SupportsNestedCancelCommand() && CancelAPI->CanCurrentlyNestedCancel())
		{
			bool bCancelled = CancelAPI->ExecuteNestedCancelCommand();
			if (bCancelled)
			{
				return;
			}
		}
	}

	const EToolShutdownType ShutdownType = GetInteractiveToolsContext()->CanCancelActiveTool() ? EToolShutdownType::Cancel : EToolShutdownType::Completed;
	GetInteractiveToolsContext()->EndTool(ShutdownType);
}


bool UHandyManEditorMode::ComputeBoundingBoxForViewportFocus(AActor* Actor, UPrimitiveComponent* PrimitiveComponent, FBox& InOutBox) const
{
	auto ProcessFocusBoxFunc = [](FBox& FocusBoxInOut)
	{
		double MaxDimension = FocusBoxInOut.GetExtent().GetMax();
		double ExpandAmount = (MaxDimension > SMALL_NUMBER) ? (MaxDimension * 0.2) : 25;		// 25 is a bit arbitrary here...
		FocusBoxInOut = FocusBoxInOut.ExpandBy(MaxDimension * 0.2);
	};

	// if Tool supports custom Focus box, use that
	if (GetToolManager()->HasAnyActiveTool())
	{
		UInteractiveTool* Tool = GetToolManager()->GetActiveTool(EToolSide::Mouse);
		IInteractiveToolCameraFocusAPI* FocusAPI = Cast<IInteractiveToolCameraFocusAPI>(Tool);
		if (FocusAPI && FocusAPI->SupportsWorldSpaceFocusBox() )
		{
			InOutBox = FocusAPI->GetWorldSpaceFocusBox();
			if (InOutBox.IsValid)
			{
				ProcessFocusBoxFunc(InOutBox);
				return true;
			}
		}
	}

	// fallback to base focus behavior
	return false;
}


bool UHandyManEditorMode::GetPivotForOrbit(FVector& OutPivot) const
{
	if (GCurrentLevelEditingViewportClient)
	{
		OutPivot = GCurrentLevelEditingViewportClient->GetViewTransform().GetLookAt();
		return true;
	}
	return false;
}



void UHandyManEditorMode::ConfigureRealTimeViewportsOverride(bool bEnable)
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();
	if (LevelEditor.IsValid())
	{
		TArray<TSharedPtr<SLevelViewport>> Viewports = LevelEditor->GetViewports();
		for (const TSharedPtr<SLevelViewport>& ViewportWindow : Viewports)
		{
			if (ViewportWindow.IsValid())
			{
				FEditorViewportClient& Viewport = ViewportWindow->GetAssetViewportClient();
				const FText SystemDisplayName = LOCTEXT("RealtimeOverrideMessage_HandyManMode", "HandyMan Mode");
				if (bEnable)
				{
					Viewport.AddRealtimeOverride(bEnable, SystemDisplayName);
				}
				else
				{
					Viewport.RemoveRealtimeOverride(SystemDisplayName, false);
				}
			}
		}
	}
}


#undef LOCTEXT_NAMESPACE
