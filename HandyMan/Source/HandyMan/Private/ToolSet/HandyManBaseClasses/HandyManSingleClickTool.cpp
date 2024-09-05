// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManBaseClasses/HandyManSingleClickTool.h"


#include "HandyManSettings.h"
#include "BaseBehaviors/MouseHoverBehavior.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "Interfaces/IPluginManager.h"

#define HDA( RelativePath, ... ) FString( UHandyManSingleClickTool::InContent(RelativePath, ".uasset" ))

// This is to fix the issue that SlateStyleMacros like IMAGE_BRUSH look for RootToContentDir but StyleSet->RootToContentDir is how this style is set up
#define RootToContentDir StyleSet->RootToContentDir

#if WITH_EDITOR
void UHandyManSingleClickTool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, ToolName) && !ToolName.IsEmpty())
	{
		// Update Our setting

		/*if (UHandyManSettings* HandyMan = GetMutableDefault<UHandyManSettings>())
		{
			if (!LastKnownName.IsEmpty())
			{
				// Remove this from the array
				HandyMan->RemoveToolName(FName(LastKnownName.ToString()));
			}

			// Add new name to the Array
			HandyMan->AddToolName(FName(ToolName.ToString()));

			// Update LastKnownName
			LastKnownName = ToolName;
		}*/
	}
}

void UHandyManSingleClickTool::PostLoad()
{
	Super::PostLoad();
}
#endif



// This is to fix the issue that SlateStyleMacros like IMAGE_BRUSH look for RootToContentDir but StyleSet->RootToContentDir is how this style is set up
#define RootToContentDir StyleSet->RootToContentDir

FString UHandyManSingleClickTool::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("HandyMan"))->GetContentDir();
	return (ContentDir / "HDA/" / RelativePath) + Extension;
}

#define LOCTEXT_NAMESPACE "UHandyManSingleClickTool"


void UHandyManSingleClickTool::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);
}


void UHandyManSingleClickTool::Setup()
{
	
	if (UHandyManSettings* HandyMan = GetMutableDefault<UHandyManSettings>())
	{
		if (!HandyMan->GetToolsNames().Contains(FName(ToolName.ToString())))
		{
			HandyMan->AddToolName(FName(ToolName.ToString()));
		}
	}
	
	UScriptableInteractiveTool::Setup();
	HandyManAPI = GEditor->GetEditorSubsystem<UHandyManSubsystem>();

	// add default button input behaviors for devices
	SingleClickBehavior = NewObject<USingleClickInputBehavior>();
	SingleClickBehavior->Initialize(this);
	AddInputBehavior(SingleClickBehavior);

	SingleClickBehavior->Modifiers.RegisterModifier(1, FInputDeviceState::IsShiftKeyDown);
	SingleClickBehavior->Modifiers.RegisterModifier(2, FInputDeviceState::IsCtrlKeyDown);
	SingleClickBehavior->Modifiers.RegisterModifier(3, FInputDeviceState::IsAltKeyDown);
	SingleClickBehavior->HitTestOnRelease = bHitTestOnRelease;
	bShiftModifier = false;
	bCtrlModifier = false;
	bAltModifier = false;


	if (bWantMouseHover)
	{
		MouseHoverBehavior = NewObject<UMouseHoverBehavior>();
		MouseHoverBehavior->Initialize(this);
		AddInputBehavior(MouseHoverBehavior);
	}

}


void UHandyManSingleClickTool::OnUpdateModifierState(int ModifierID, bool bIsOn)
{
	if (ModifierID == 1)
	{
		bShiftModifier = bIsOn;
	}
	if (ModifierID == 2)
	{
		bCtrlModifier = bIsOn;
	}
	if (ModifierID == 3)
	{
		bAltModifier = bIsOn;
	}
}

bool UHandyManSingleClickTool::IsShiftDown() const
{
	return bShiftModifier;
}

bool UHandyManSingleClickTool::IsCtrlDown() const
{
	return bCtrlModifier;
}

bool UHandyManSingleClickTool::IsAltDown() const
{
	return bAltModifier;
}

FScriptableToolModifierStates UHandyManSingleClickTool::GetActiveModifiers()
{
	FScriptableToolModifierStates Modifiers;
	Modifiers.bShiftDown = bShiftModifier;
	Modifiers.bCtrlDown = bCtrlModifier;
	Modifiers.bAltDown = bAltModifier;
	return Modifiers;
}


FInputRayHit UHandyManSingleClickTool::TestIfHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	UE_LOG(LogTemp, Warning, TEXT("UHandyManSingleClickTool: Default TestIfHitByClick Implementation always consumes hit"));

	// always return a hit
	return FInputRayHit(0.0f);
}
FInputRayHit UHandyManSingleClickTool::IsHitByClick(const FInputDeviceRay& ClickPos)
{
	return TestIfHitByClick(ClickPos, GetActiveModifiers());
}

void UHandyManSingleClickTool::OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	UE_LOG(LogTemp, Warning, TEXT("UHandyManSingleClickTool: Empty OnHitByClick Implementation"));
}
void UHandyManSingleClickTool::OnClicked(const FInputDeviceRay& ClickPos)
{
	OnHitByClick(ClickPos, GetActiveModifiers());
}


FInputRayHit UHandyManSingleClickTool::OnHoverHitTest_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	// always return a hit
	return FInputRayHit(0.0f);
}
FInputRayHit UHandyManSingleClickTool::BeginHoverSequenceHitTest(const FInputDeviceRay& ClickPos)
{
	return OnHoverHitTest(ClickPos, GetActiveModifiers());
}

void UHandyManSingleClickTool::OnHoverBegin_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers)
{
}

void UHandyManSingleClickTool::OnBeginHover(const FInputDeviceRay& DevicePos)
{
	bInHover = true;
	OnHoverBegin(DevicePos, GetActiveModifiers());
}

bool UHandyManSingleClickTool::OnHoverUpdate_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers)
{
	return true;
}

bool UHandyManSingleClickTool::OnUpdateHover(const FInputDeviceRay& DevicePos)
{
	return OnHoverUpdate(DevicePos, GetActiveModifiers());
}


void UHandyManSingleClickTool::OnHoverEnd_Implementation(const FScriptableToolModifierStates& Modifiers)
{
}

void UHandyManSingleClickTool::OnEndHover()
{
	OnHoverEnd(GetActiveModifiers());
	bInHover = false;
}

bool UHandyManSingleClickTool::InActiveHover() const
{
	return bInHover;
}


#undef LOCTEXT_NAMESPACE

