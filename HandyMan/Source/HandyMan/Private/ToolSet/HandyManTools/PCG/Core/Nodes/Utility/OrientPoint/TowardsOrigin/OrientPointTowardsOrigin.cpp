// Fill out your copyright notice in the Description page of Project Settings.


#include "OrientPointTowardsOrigin.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"
#include "Helpers/PCGAsync.h"

#define LOCTEXT_NAMESPACE "OrientPointsTowardOrigin"

UOrientPointTowardsOriginSettings::UOrientPointTowardsOriginSettings()
{
	AxisToKeep.Add(EAxisToKeep::Pitch, true);
	AxisToKeep.Add(EAxisToKeep::Roll, true);
	AxisToKeep.Add(EAxisToKeep::Yaw, true);
}

TArray<FPCGPinProperties> UOrientPointTowardsOriginSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;

	FPCGPinProperties SourcePin;
	SourcePin.Label = FName("Source");
	SourcePin.AllowedTypes = EPCGDataType::Point;
	SourcePin.PinStatus = EPCGPinStatus::Required;

	FPCGPinProperties TargetPin;
	TargetPin.Label = FName("Origin");
	TargetPin.AllowedTypes = EPCGDataType::Point;
	TargetPin.PinStatus = EPCGPinStatus::Required;

	Properties.Emplace(SourcePin);
	Properties.Emplace(TargetPin);
	return Properties;
}

FPCGElementPtr UOrientPointTowardsOriginSettings::CreateElement() const
{
	return MakeShared<FPCGOrientPointsToOriginElement>();
}


/// ------------------------------------------------------------------------
/// POINT LOOP

bool FPCGOrientPointsToOriginElement::ExecuteInternal(FPCGContext* Context) const
{
	const UOrientPointTowardsOriginSettings* Settings = Context->GetInputSettings<UOrientPointTowardsOriginSettings>();
	check(Settings);

	const TArray<FPCGTaggedData> SourceInputs = Context->InputData.GetInputsByPin(FName("Source"));
	const TArray<FPCGTaggedData> TargetInputs = Context->InputData.GetInputsByPin(FName("Origin"));
	
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;
	
	const FPCGTaggedData& TargetPoint = TargetInputs[0];
	const UPCGPointData* TargetPointData = Cast<UPCGPointData>(TargetPoint.Data);
	const FPCGPoint& Target = TargetPointData->GetPoint(0);





	// Use implicit capture, since we capture a lot
	//ProcessPoints(Context, Inputs, Outputs, [&](const FPCGPoint& InPoint, FPCGPoint& OutPoint)

	TMap<EAxisToKeep, bool> AxisToKeep = Settings->AxisToKeep;

	const bool bShouldDebug = Settings->bDebug;

	for (const auto& Input : SourceInputs)
	{

		TRACE_CPUPROFILER_EVENT_SCOPE(FPCGOrientPointsElement::Execute::InputLoop);
	
		FPCGTaggedData& Output = Outputs.Add_GetRef(Input);
		
		const UPCGPointData* SourcePointData = Cast<UPCGPointData>(Input.Data);

		if (!SourcePointData || !TargetPointData )
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

			FRotator LookAtRot = (Target.Transform.GetLocation() - OutPoint.Transform.GetLocation()).Rotation();
			for (auto Axis : AxisToKeep)
			{
			
				if (Axis.Key == EAxisToKeep::Pitch)
				{
					LookAtRot.Pitch = Axis.Value ? LookAtRot.Pitch : 0.f;
				}
				else if (Axis.Key == EAxisToKeep::Roll)
				{
					LookAtRot.Roll = Axis.Value ? LookAtRot.Roll : 0.f;
				}
				else
				{
					LookAtRot.Yaw = Axis.Value ? LookAtRot.Yaw : 0.f;
				}
			}
		
			OutPoint.Transform.SetRotation(LookAtRot.Quaternion());

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