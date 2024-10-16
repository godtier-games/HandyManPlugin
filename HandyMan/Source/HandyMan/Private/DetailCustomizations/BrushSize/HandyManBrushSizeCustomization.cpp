﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "HandyManBrushSizeCustomization.h"

#include "DetailWidgetRow.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBox.h"
#include "ModelingWidgets/ModelingCustomizationUtil.h"
#include "ToolSet/HandyManTools/Core/SculptTool/DataTypes/HandyManSculptingTypes.h"
#include "Widgets/SBoxPanel.h"

using namespace UE::ModelingUI;

#define LOCTEXT_NAMESPACE "ModelingToolsBrushSizeCustomization"



TSharedRef<IPropertyTypeCustomization> FHandyManBrushSizeCustomization::MakeInstance()
{
	return MakeShareable(new FHandyManBrushSizeCustomization);
}

void FHandyManBrushSizeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> SizeType = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHandyManBrushToolRadius, SizeType));
	SizeType->MarkHiddenByCustomization();

	TSharedPtr<IPropertyHandle> AdaptiveSize = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHandyManBrushToolRadius, AdaptiveSize));
	AdaptiveSize->MarkHiddenByCustomization();

	TSharedPtr<IPropertyHandle> WorldRadius = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHandyManBrushToolRadius, WorldRadius));
	WorldRadius->MarkHiddenByCustomization();

	auto GetCurrentSizeType = [SizeType]() -> EHandyManBrushToolSizeType
	{
		uint8 SizeTypeInt = 0;
		SizeType->GetValue(SizeTypeInt);
		return (SizeTypeInt == (uint8)EHandyManBrushToolSizeType::World) ? EHandyManBrushToolSizeType::World : EHandyManBrushToolSizeType::Adaptive;
	};
	auto GetSizeHandle = [SizeType, AdaptiveSize, WorldRadius]() -> TSharedPtr<IPropertyHandle>
	{
		uint8 SizeTypeInt = 0;
		SizeType->GetValue(SizeTypeInt);
		return (SizeTypeInt == (uint8)EHandyManBrushToolSizeType::World) ? WorldRadius : AdaptiveSize;
	};

	TSharedPtr<SDynamicNumericEntry::FDataSource> NumericSource = MakeShared<SDynamicNumericEntry::FDataSource>();
	NumericSource->SetValue = [GetSizeHandle](float NewSize, EPropertyValueSetFlags::Type Flags)
	{
		GetSizeHandle()->SetValue(NewSize, Flags);
	};
	NumericSource->GetValue = [GetSizeHandle]() -> float
	{
		float Size;
		GetSizeHandle()->GetValue(Size);
		return Size;
	};
	NumericSource->GetValueRange = [GetCurrentSizeType]() -> TInterval<float>
	{
		return (GetCurrentSizeType() == EHandyManBrushToolSizeType::Adaptive) ? TInterval<float>(0.0f, 10.0f) : TInterval<float>(0.01f, 50000.0f);
	};
	NumericSource->GetUIRange = [GetCurrentSizeType]() -> TInterval<float>
	{
		return (GetCurrentSizeType() == EHandyManBrushToolSizeType::Adaptive) ? TInterval<float>(0.0f, 1.0f) : TInterval<float>(1.0f, 1000.0f);
	};

	HeaderRow
		.OverrideResetToDefault(FResetToDefaultOverride::Hide())
		.WholeRowContent()
	[
		SNew(SHorizontalBox)

		// name
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(0,4))
		[
			WrapInFixedWidthBox(AdaptiveSize->CreatePropertyNameWidget(), FHandyManSculptToolsUIConstants::SculptShortLabelWidth)
		]

		// slider
		+ SHorizontalBox::Slot()
		.Padding(ModelingUIConstants::LabelWidgetMinPadding, 0,0,0)
		.FillWidth(10.0f)
		[
			SNew(SDynamicNumericEntry)
			.Source(NumericSource)
		]

		// toggle
		+ SHorizontalBox::Slot()
		.AutoWidth()
		//.FillWidth(0.5f)
		.Padding(FMargin(ModelingUIConstants::MultiWidgetRowHorzPadding, ModelingUIConstants::DetailRowVertPadding, 0, ModelingUIConstants::DetailRowVertPadding))
		.HAlign(HAlign_Right)
		[
			SNew(SBox)
			.WidthOverride(50)
			[
				SNew(SCheckBox)
					.Style(FAppStyle::Get(), "DetailsView.SectionButton")
					.Padding(FMargin(4, 2))
					.HAlign(HAlign_Center)
					.ToolTipText( LOCTEXT("WorldToggleTooltip", "Specify Brush Size in World Units") )
					.OnCheckStateChanged_Lambda([this, SizeType](ECheckBoxState State) {
						SizeType->SetValue( static_cast<uint8>( (State == ECheckBoxState::Checked) ? EHandyManBrushToolSizeType::World : EHandyManBrushToolSizeType::Adaptive ) );
					})
					.IsChecked_Lambda([this, SizeType]() {
						uint8 CurValueInt;
						SizeType->GetValue(CurValueInt);
						return (CurValueInt == static_cast<uint8>(EHandyManBrushToolSizeType::World)) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(FMargin(0))
						.AutoWidth()
						[
							SNew(STextBlock)
							.Justification(ETextJustify::Center)
							.TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
							.Text( LOCTEXT("World", "World") )
						]
					]
			]
		]
	];

}


void FHandyManBrushSizeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}


#undef LOCTEXT_NAMESPACE
