// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/HandyManGroupSetCustomization.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "UI/HandyManToolGroupCombo.h"
#include "Widgets/Layout/SBox.h"

#define LOCTEXT_NAMESPACE "FScriptableToolTrackGroupSetCustomization"

TSharedRef<IPropertyTypeCustomization> FHandyManGroupSetCustomization::MakeInstance()
{
	return MakeShareable(new FHandyManGroupSetCustomization);
}

void FHandyManGroupSetCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow
		.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
	.ValueContent()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			[
				SNew(HandyManToolGroupSetCombo)
				.StructPropertyHandle(StructPropertyHandle)
			]
		];
}

void FHandyManGroupSetCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{}

#undef LOCTEXT_NAMESPACE
