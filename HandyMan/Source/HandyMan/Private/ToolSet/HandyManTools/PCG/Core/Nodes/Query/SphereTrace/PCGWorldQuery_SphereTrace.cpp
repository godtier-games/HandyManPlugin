// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGWorldQuery_SphereTrace.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "Data/PCGPointData.h"

#define LOCTEXT_NAMESPACE "PCGWorldQuery_SphereTrace"
namespace PCGSphereTraceSettings
{
	static const FName PointsLabel = TEXT("TransformedPoint");
    
}


UPCGWorldQuery_SphereTraceSettings::UPCGWorldQuery_SphereTraceSettings()
{
	
}

TArray<FPCGPinProperties> UPCGWorldQuery_SphereTraceSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;

	FPCGPinProperties SourcePin;
	SourcePin.Label = FName("Source");
	SourcePin.AllowedTypes = EPCGDataType::Point;
	SourcePin.PinStatus = EPCGPinStatus::Required;

	FPCGPinProperties TargetPin;
	TargetPin.Label = FName("ReferencePoint");
	TargetPin.AllowedTypes = EPCGDataType::Point;
	TargetPin.Tooltip = LOCTEXT("Tooltip", "Optional point of reference for the final orientation of the point. If the point is not looking towards this point it will change its orientation to do so");

	Properties.Emplace(SourcePin);
	Properties.Emplace(TargetPin);
	return Properties;
}

TArray<FPCGPinProperties> UPCGWorldQuery_SphereTraceSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;

	Properties.Emplace(PCGSphereTraceSettings::PointsLabel, EPCGDataType::Point);

	return Properties;
}

FPCGElementPtr UPCGWorldQuery_SphereTraceSettings::CreateElement() const
{
	return MakeShared<FPCGSphereTraceElement>();
}

/// ---------------------------------------------------------
/// POINT LOOP
bool FPCGSphereTraceElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGFilterBySeedValueElement::Execute);

	check(Context);

	const UPCGWorldQuery_SphereTraceSettings* Settings = Context->GetInputSettings<UPCGWorldQuery_SphereTraceSettings>();
	check(Settings);


	const TArray<FPCGTaggedData>& OptionalLookAtPoint = Context->InputData.GetInputsByPin(FName("ReferencePoint"));

	bool bShouldOrientPoint = OptionalLookAtPoint.Num() > 0;

	FVector OptionalLookAtLocation;
	if (bShouldOrientPoint)
	{
		if (const UPCGPointData* OptionalLookAtPointData = Cast<UPCGPointData>(OptionalLookAtPoint[0].Data))
		{
			OptionalLookAtLocation = OptionalLookAtPointData->GetPoint(0).Transform.GetRotation().Vector().GetSafeNormal();
		}
	}
	
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;
	
	const UWorld* World = Context->SourceComponent.Get()->GetWorld();
	check(World);

	for (const FPCGTaggedData& Input : Context->InputData.GetInputsByPin(FName("Source")))
	{
		const UPCGPointData* PointData = Cast<UPCGPointData>(Input.Data);

		if (!PointData)
		{
			PCGE_LOG(Error, GraphAndLog, LOCTEXT("InputNotPointData", "Input is not a point data"));
			continue;
		}

		const TArray<FPCGPoint>& InPoints = PointData->GetPoints();

		// Create output point data. This is the recommended way instead of writing directly to the incoming points.
		UPCGPointData* ChosenPointsData = NewObject<UPCGPointData>();
		ChosenPointsData->InitializeFromData(PointData);
		TArray<FPCGPoint>& ChosenPoints = ChosenPointsData->GetMutablePoints();
		FRandomStream RandomStream(Settings->Seed);
		
		TArray<FHitResult> OutHits;
        
		for (int i =0; i < InPoints.Num(); i++)
		{
			FVector TraceLocation = InPoints[i].Transform.GetLocation();
			TraceLocation.Z += Settings->TraceHeightOffset;
			if (Trace(World, TraceLocation, TraceLocation, Settings->ChannelsToTrace, Settings->SphereRadius, OutHits))
			{
				FPCGPoint AlteredPoint = InPoints[i];
				const FRotator OriginalRot = AlteredPoint.Transform.GetRotation().Rotator();
				FTransform PointTransform = AlteredPoint.Transform;
				switch (Settings->TransformResolution)
				{
				case ETransformResolutionType::First:
					{
						if (OutHits[0].Normal.Equals(FVector(0,0,1)))
						{
							continue;
						}
						
						FRotator ReorientedNormal = OutHits[0].Normal.Rotation();
						if (bShouldOrientPoint)
						{
							const auto Dot = FVector::DotProduct(OutHits[0].Normal, OptionalLookAtLocation);
							if (Dot < 0 && FMath::IsNearlyZero(FMath::Abs(Dot)))
							{
								ReorientedNormal = (-OutHits[0].Normal).Rotation();
							}
						}

						
						FRotator PointRot = Settings->bInheritSurfaceRotation ? ReorientedNormal : OriginalRot;
						PointTransform = FTransform(PointRot, OutHits[0].Location);
					}
					break;
				case ETransformResolutionType::Last:
					{
						if (OutHits.Last().Normal.Equals(FVector(0,0,1)))
						{
							continue;
						}
						
						FRotator ReorientedNormal = OutHits[0].Normal.Rotation();
						if (bShouldOrientPoint)
						{
							const auto Dot = FVector::DotProduct(OutHits[0].Normal, OptionalLookAtLocation);
							if (Dot < 0 && FMath::IsNearlyZero(FMath::Abs(Dot)))
							{
								ReorientedNormal = (-OutHits[0].Normal).Rotation();
							}
						}

						
						FRotator PointRot = Settings->bInheritSurfaceRotation ? ReorientedNormal : OriginalRot;
						PointTransform = FTransform(PointRot, OutHits.Last().Location);
					}
					break;
				case ETransformResolutionType::Random:
					{
						const int32 RandIndex = RandomStream.RandRange(0, OutHits.Num() - 1);
						if (OutHits[RandIndex].Normal.Equals(FVector(0,0,1)))
						{
							continue;
						}
						
						FRotator ReorientedNormal = OutHits[0].Normal.Rotation();
						if (bShouldOrientPoint)
						{
							const auto Dot = FVector::DotProduct(OutHits[0].Normal, OptionalLookAtLocation);
							if (Dot < 0 && FMath::IsNearlyZero(FMath::Abs(Dot)))
							{
								ReorientedNormal = (-OutHits[0].Normal).Rotation();
							}
						}
						
						FRotator PointRot = Settings->bInheritSurfaceRotation ? ReorientedNormal : OriginalRot;
						PointTransform = FTransform(PointRot, OutHits[RandIndex].Location);
					}
					break;
				}
				
				AlteredPoint.Transform = PointTransform;
				ChosenPoints.Add(AlteredPoint);


#if ENABLE_DRAW_DEBUG
				if (Settings->bDebug)
				{
					DrawDebugSphere(World, PointTransform.GetLocation(), Settings->SphereRadius, 12, FColor::Red, false, 1.f, 0, 1.25f);
					DrawDebugBox(World, PointTransform.GetLocation(), FVector(10.f), FColor::Green, false, 1.f, 0, 1.25f);
				}
#endif
			}
		}
        
        
		// Output all in output collection
		FPCGTaggedData& ChosenTaggedData = Outputs.Add_GetRef(Input);
		ChosenTaggedData.Data = ChosenPointsData;
		ChosenTaggedData.Pin = PCGSphereTraceSettings::PointsLabel;

		
	}

	return true;
}

bool FPCGSphereTraceElement::Trace(const UWorld* World, const FVector& Start, const FVector& End, const TArray<TEnumAsByte<ECollisionChannel>>& Types, const float& Radius, TArray<FHitResult>& OutHits) const
{
	if (!World) return false;

	FCollisionObjectQueryParams Params;
	for (const auto& Type : Types)
	{
		Params.AddObjectTypesToQuery(Type);
	}
    
	return World->SweepMultiByObjectType(OutHits, Start, End, FQuat::Identity, Params, FCollisionShape::MakeSphere(Radius));
}


#undef LOCTEXT_NAMESPACE