// Fill out your copyright notice in the Description page of Project Settings.


#include "HandyManEditorModeStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"


#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FHandyManEditorModeStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )
#define IMAGE_PLUGIN_SVG( RelativePath, ... ) FSlateVectorImageBrush( FHandyManEditorModeStyle::InContent( RelativePath, ".svg" ), __VA_ARGS__ )

// This is to fix the issue that SlateStyleMacros like IMAGE_BRUSH look for RootToContentDir but StyleSet->RootToContentDir is how this style is set up
#define RootToContentDir StyleSet->RootToContentDir

FString FHandyManEditorModeStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("HandyMan"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

TSharedPtr< FSlateStyleSet > FHandyManEditorModeStyle::StyleSet = nullptr;
TSharedPtr< class ISlateStyle > FHandyManEditorModeStyle::Get() { return StyleSet; }

FName FHandyManEditorModeStyle::GetStyleSetName()
{
	static FName HandyManEditorModeStyleName(TEXT("HandyManEditorModeStyle"));
	return HandyManEditorModeStyleName;
}

const FSlateBrush* FHandyManEditorModeStyle::GetBrush(FName PropertyName, const ANSICHAR* Specifier)
{
	return Get()->GetBrush(PropertyName, Specifier);
}


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FHandyManEditorModeStyle::Initialize()
{
	// Const icon sizes
	const FVector2D Icon8x8(8.0f, 8.0f);
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon28x28(28.0f, 28.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon120(120.0f, 120.0f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(FPaths::ProjectPluginsDir() / TEXT("HandyMan/Content"));
	StyleSet->SetCoreContentRoot(FPaths::ProjectPluginsDir() / TEXT("Slate"));

	const FTextBlockStyle& NormalText = FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");

	{
		StyleSet->Set("LevelEditor.HandyManEditorMode", new IMAGE_PLUGIN_BRUSH("Icons/HandyManEditorMode_Icon_40x", FVector2D(20.0f, 20.0f)));

		StyleSet->Set("HandyManEditorModeToolCommands.DefaultToolIcon", new IMAGE_PLUGIN_BRUSH("Icons/Tool_DefaultIcon_40px", Icon20x20));
		StyleSet->Set("HandyManEditorModeToolCommands.DefaultToolIcon.Small", new IMAGE_PLUGIN_BRUSH("Icons/Tool_DefaultIcon_40px", Icon20x20));



		//
		// Icons for brush falloffs in sculpt/etc tools
		//

		StyleSet->Set("BrushFalloffIcons.Smooth", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Falloff_Smooth", Icon120));
		StyleSet->Set("BrushFalloffIcons.Linear", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Falloff_Linear", Icon120));
		StyleSet->Set("BrushFalloffIcons.Inverse", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Falloff_Inverse", Icon120));
		StyleSet->Set("BrushFalloffIcons.Round", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Falloff_Round", Icon120));
		StyleSet->Set("BrushFalloffIcons.BoxSmooth", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Falloff_BoxSmooth", Icon120));
		StyleSet->Set("BrushFalloffIcons.BoxLinear", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Falloff_BoxLinear", Icon120));
		StyleSet->Set("BrushFalloffIcons.BoxInverse", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Falloff_BoxInverse", Icon120));
		StyleSet->Set("BrushFalloffIcons.BoxRound", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Falloff_BoxRound", Icon120));


		//
		// Icons for brushes in sculpt/etc tools
		//

		StyleSet->Set("BrushTypeIcons.Smooth", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_Smooth", Icon120));
		StyleSet->Set("BrushTypeIcons.SmoothFill", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_SmoothFill", Icon120));
		StyleSet->Set("BrushTypeIcons.Move", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_Move", Icon120));
		StyleSet->Set("BrushTypeIcons.SculptN", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_SculptN", Icon120));
		StyleSet->Set("BrushTypeIcons.SculptV", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_SculptV", Icon120));
		StyleSet->Set("BrushTypeIcons.SculptMx", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_SculptMx", Icon120));
		StyleSet->Set("BrushTypeIcons.Inflate", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_Inflate", Icon120));
		StyleSet->Set("BrushTypeIcons.Pinch", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_Pinch", Icon120));
		StyleSet->Set("BrushTypeIcons.Flatten", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_Flatten", Icon120));
		StyleSet->Set("BrushTypeIcons.PlaneN", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_PlaneN", Icon120));
		StyleSet->Set("BrushTypeIcons.PlaneV", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_PlaneV", Icon120));
		StyleSet->Set("BrushTypeIcons.PlaneW", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_PlaneW", Icon120));
		StyleSet->Set("BrushTypeIcons.Scale", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_Scale", Icon120));
		StyleSet->Set("BrushTypeIcons.Grab", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_Grab", Icon120));
		StyleSet->Set("BrushTypeIcons.GrabSharp", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_GrabSharp", Icon120));
		StyleSet->Set("BrushTypeIcons.Twist", new IMAGE_PLUGIN_SVG("Icons/BrushIcons/Brush_Twist", Icon120));

	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
};


bool FHandyManEditorModeStyle::TryRegisterCustomIcon(FName StyleIdentifier, FString FileNameWithPath, FString ExternalRelativePath)
{
	if ( ! ensure(StyleSet.IsValid()) )
	{
		return false;
	}

	const FVector2D Icon20x20(20.0f, 20.0f);

	if (ExternalRelativePath.IsEmpty())
	{
		ExternalRelativePath = FileNameWithPath;
	}

	if (FPaths::FileExists(FileNameWithPath) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Custom ScriptableTool Icon file not found at path %s (on-disk path is %s)"), *ExternalRelativePath, *FileNameWithPath);
		return false;
	}

	FString Extension = FPaths::GetExtension(FileNameWithPath);
	if ( !(Extension.Equals("svg", ESearchCase::IgnoreCase) || Extension.Equals("png", ESearchCase::IgnoreCase)) )
	{
		UE_LOG(LogTemp, Warning, TEXT("Custom ScriptableTool Icon at path %s has unsupported type, must be svg or png"), *ExternalRelativePath);
		return false;
	}

	// need to re-register style to be able to modify it
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());

	if (Extension.Equals("svg", ESearchCase::IgnoreCase))
	{
		StyleSet->Set(StyleIdentifier,
			new FSlateVectorImageBrush(FileNameWithPath, Icon20x20));
	}
	else if (Extension.Equals("png", ESearchCase::IgnoreCase))
	{
		StyleSet->Set(StyleIdentifier,
			new FSlateImageBrush(FileNameWithPath, Icon20x20) );
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());

	return true;
}



END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef IMAGE_PLUGIN_BRUSH
#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef DEFAULT_FONT

void FHandyManEditorModeStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}
