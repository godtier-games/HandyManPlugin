// Fill out your copyright notice in the Description page of Project Settings.


#include "OrientPointSettings.h"
#include "PCGContext.h"
#include "PCGModule.h"
#include "Data/PCGPointData.h"
#include "Helpers/PCGAsync.h"
#include "Helpers/PCGHelpers.h"
#include "Kismet/KismetMathLibrary.h"

#define LOCTEXT_NAMESPACE "PCGTransformPointsElement"

UOrientPointSettings::UOrientPointSettings()
{
}

FPCGElementPtr UOrientPointSettings::CreateElement() const
{
	return MakeShared<FPCGOrientPointsElement>();
}

bool FPCGOrientPointsElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGOrientPointsElement::Execute);

	const UOrientPointSettings* Settings = Context->GetInputSettings<UOrientPointSettings>();
	check(Settings);

	const TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputs();
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	const float InMeshOffsetDistance = Settings->MeshDistanceOffset;
	const int Seed = Context->GetSeed();

	// Use implicit capture, since we capture a lot
	//ProcessPoints(Context, Inputs, Outputs, [&](const FPCGPoint& InPoint, FPCGPoint& OutPoint)
	for (const FPCGTaggedData& Input : Inputs)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FPCGOrientPointsElement::Execute::InputLoop);
		FPCGTaggedData& Output = Outputs.Add_GetRef(Input);

		const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Input.Data);

		if (!SpatialData)
		{
			PCGE_LOG(Error, GraphAndLog, LOCTEXT("InputMissingSpatialData", "Unable to get Spatial data from input"));
			continue;
		}

		const UPCGPointData* PointData = SpatialData->ToPointData(Context);

		if (!PointData)
		{
			PCGE_LOG(Error, GraphAndLog, LOCTEXT("InputMissingPointData", "Unable to get Point data from input"));
			continue;
		}
		

		const TArray<FPCGPoint>& Points = PointData->GetPoints();

		UPCGPointData* OutputData = NewObject<UPCGPointData>();
		OutputData->InitializeFromData(PointData);
		TArray<FPCGPoint>& OutputPoints = OutputData->GetMutablePoints();
		Output.Data = OutputData;

		
		TArray<TTuple<int64, int64>> AllMetadataEntries;
		
		FPCGAsync::AsyncPointProcessing(Context, Points.Num(), OutputPoints, [&](int32 Index, FPCGPoint& OutPoint)
		{
			
			if (Index + 1 < Points.Num())
			{
				const FPCGPoint& InPoint = Points[Index];
				const FPCGPoint& NextPoint = Points[(Index + 1)];
				OutPoint = InPoint;

				const FVector& TravelVector = NextPoint.Transform.GetLocation() - InPoint.Transform.GetLocation();
				const FRotator& LookAtRotation = UKismetMathLibrary::FindLookAtRotation(InPoint.Transform.GetLocation(), NextPoint.Transform.GetLocation());
				const FVector& AdjustedScale = FVector(TravelVector.Length() / InMeshOffsetDistance == 0 ? 1 : InMeshOffsetDistance, InPoint.Transform.GetScale3D().Y, InPoint.Transform.GetScale3D().Z);
				OutPoint.Transform.SetRotation(LookAtRotation.Quaternion());
				OutPoint.Transform.SetScale3D(AdjustedScale);

				return true;
			}
			

			
			return false;
		});

	}

	return true;
}

#undef LOCTEXT_NAMESPACE
