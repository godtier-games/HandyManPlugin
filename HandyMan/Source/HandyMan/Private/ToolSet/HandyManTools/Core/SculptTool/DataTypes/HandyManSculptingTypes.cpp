// Fill out your copyright notice in the Description page of Project Settings.


#include "HandyManSculptingTypes.h"

void FHandyManBrushToolRadius::InitializeWorldSizeRange(TInterval<float> Range, bool bValidateWorldRadius)
{
	WorldSizeRange = Range;
	if (WorldRadius < WorldSizeRange.Min)
	{
		WorldRadius = WorldSizeRange.Interpolate(0.2);
	}
	else if (WorldRadius > WorldSizeRange.Max)
	{
		WorldRadius = WorldSizeRange.Interpolate(0.8);
	}
}

float FHandyManBrushToolRadius::GetWorldRadius() const
{
	if (SizeType == EHandyManBrushToolSizeType::Adaptive)
	{
		return 0.5 * WorldSizeRange.Interpolate( FMath::Max(0, AdaptiveSize) );
	}
	else
	{
		return WorldRadius;
	}
}

void FHandyManBrushToolRadius::IncreaseRadius(bool bSmallStep)
{
	float StepSize = (bSmallStep) ? 0.005f : 0.025f;
	if (SizeType == EHandyManBrushToolSizeType::Adaptive)
	{
		AdaptiveSize = FMath::Clamp(AdaptiveSize + StepSize, 0.0f, 1.0f);
	}
	else
	{
		float dt = StepSize * 0.5 * WorldSizeRange.Size();
		WorldRadius = FMath::Clamp(WorldRadius + dt, WorldSizeRange.Min, WorldSizeRange.Max);
	}
}

void FHandyManBrushToolRadius::DecreaseRadius(bool bSmallStep)
{
	float StepSize = (bSmallStep) ? 0.005f : 0.025f;
	if (SizeType == EHandyManBrushToolSizeType::Adaptive)
	{
		AdaptiveSize = FMath::Clamp(AdaptiveSize - StepSize, 0.0f, 1.0f);
	}
	else
	{
		float dt = StepSize * 0.5 * WorldSizeRange.Size();
		WorldRadius = FMath::Clamp(WorldRadius - dt, WorldSizeRange.Min, WorldSizeRange.Max);
	}
}
