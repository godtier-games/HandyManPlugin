// Fill out your copyright notice in the Description page of Project Settings.


#include "InvertPointNormalSettings.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGModule.h"
#include "Data/PCGPointData.h"
#include "Helpers/PCGAsync.h"


#define LOCTEXT_NAMESPACE "OrientPointsTowardOrigin"

class UPCGPointData;

UInvertPointNormalSettings::UInvertPointNormalSettings()
{
}

TArray<FPCGPinProperties> UInvertPointNormalSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;

	FPCGPinProperties SourcePin;
	SourcePin.Label = FName("Source");
	SourcePin.AllowedTypes = EPCGDataType::Point;
	SourcePin.PinStatus = EPCGPinStatus::Required;
	
	Properties.Emplace(SourcePin);
	return Properties;
}

FPCGElementPtr UInvertPointNormalSettings::CreateElement() const
{
	return MakeShared<FPCGInvertPointNormalsElement>();
}


/// ------------------------------------------------------------------------
/// POINT LOOP

bool FPCGInvertPointNormalsElement::ExecuteInternal(FPCGContext* Context) const
{
	const UInvertPointNormalSettings* Settings = Context->GetInputSettings<UInvertPointNormalSettings>();
	check(Settings);

	const TArray<FPCGTaggedData> SourceInputs = Context->InputData.GetInputsByPin(FName("Source"));
	const TArray<FPCGTaggedData> TargetInputs = Context->InputData.GetInputsByPin(FName("Origin"));
	
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;




	// Use implicit capture, since we capture a lot
	//ProcessPoints(Context, Inputs, Outputs, [&](const FPCGPoint& InPoint, FPCGPoint& OutPoint)


	const bool bShouldDebug = Settings->bDebug;

	for (const auto& Input : SourceInputs)
	{

		TRACE_CPUPROFILER_EVENT_SCOPE(FPCGOrientPointsElement::Execute::InputLoop);
	
		FPCGTaggedData& Output = Outputs.Add_GetRef(Input);
		
		const UPCGPointData* SourcePointData = Cast<UPCGPointData>(Input.Data);

		if (!SourcePointData  )
		{
			PCGE_LOG(Error, GraphAndLog, LOCTEXT("InputMissingPointData", "Unable to get Point data from inputs"));
			return false;
		}
		

		const TArray<FPCGPoint>& PointsToOrient = SourcePointData->GetPoints();

		UPCGPointData* OutputData = NewObject<UPCGPointData>();
		OutputData->InitializeFromData(SourcePointData);
		TArray<FPCGPoint>& OutputPoints = OutputData->GetMutablePoints();
		Output.Data = OutputData;

		
		TArray<TTuple<int64, int64>> AllMetadataEntries;
		
		FPCGAsync::AsyncPointProcessing(Context, PointsToOrient.Num(), OutputPoints, [&](int32 Index, FPCGPoint& OutPoint)
		{
		
			OutPoint = PointsToOrient[Index];

			const FVector Inverted = -(OutPoint.Transform.Rotator().Vector().GetSafeNormal());
		
			OutPoint.Transform.SetRotation(Inverted.Rotation().Quaternion());

#if ENABLE_DRAW_DEBUG
			if (bShouldDebug)
			{
				FVector Start = OutPoint.Transform.GetLocation();
				FVector End = Start + OutPoint.Transform.GetRotation().Vector() * 100.f;
				DrawDebugDirectionalArrow(Context->SourceComponent.Get()->GetWorld(), Start , End, 20.f, FColor::Blue, false, 2, 0, 1.25 );
			}
#endif
		
		
			return true;
		
		});
		
	}
	return true;
}

#undef LOCTEXT_NAMESPACE