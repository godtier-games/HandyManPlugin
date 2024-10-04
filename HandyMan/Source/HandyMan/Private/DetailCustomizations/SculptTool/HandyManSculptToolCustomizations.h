// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "IDetailCustomization.h"
#include "UObject/WeakObjectPtr.h"

class FReply;
class SButton;
class SWidget;
enum class ECheckBoxState : uint8;

class IDetailLayoutBuilder;
class IPropertyHandle;
class IDetailChildrenBuilder;
class SComboButton;
class SComboPanel;
class SBox;
class UMorphTargetCreator;
class FRecentAlphasProvider;
class SToolInputAssetComboPanel;


// customization for USculptBrushProperties, creates two-column layout
// for secondary brush properties like lazy/etc
class FHandyManSculptBrushPropertiesDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};


// customization for vertexsculpt properties, creates combopanel for brush type,
// small-style combopanel for falloff type, and stacks controls to the right
class FHandyManVertexBrushSculptPropertiesDetails : public IDetailCustomization
{
public:
	virtual ~FHandyManVertexBrushSculptPropertiesDetails();

	static TSharedRef<IDetailCustomization> MakeInstance();
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

protected:
	TWeakObjectPtr<UMorphTargetCreator> TargetTool;

	TSharedPtr<SWidget> MakeRegionFilterWidget();
	TSharedPtr<SWidget> MakeFreezeTargetWidget();
	
	TSharedPtr<SButton> FreezeTargetButton;
	FReply OnToggledFreezeTarget();
	void OnSetFreezeTarget(ECheckBoxState State);
	bool IsFreezeTargetEnabled();

	TSharedPtr<SComboPanel> FalloffTypeCombo;
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	FDelegateHandle FalloffTypeUpdateHandle;
};



// customization for UVertexBrushAlphaProperties, creates custom asset picker
// tileview-combopanel for brush alphas and stacks controls to the right
class FHandyManVertexBrushAlphaPropertiesDetails : public IDetailCustomization
{
public:
	virtual ~FHandyManVertexBrushAlphaPropertiesDetails();

	static TSharedRef<IDetailCustomization> MakeInstance();
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	TSharedPtr<FRecentAlphasProvider> RecentAlphasProvider;

protected:
	TWeakObjectPtr<UMorphTargetCreator> TargetTool;
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	TSharedPtr<SToolInputAssetComboPanel> AlphaAssetPicker;
	FDelegateHandle AlphaTextureUpdateHandle;
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "IPropertyTypeCustomization.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Views/STileView.h"
#endif

