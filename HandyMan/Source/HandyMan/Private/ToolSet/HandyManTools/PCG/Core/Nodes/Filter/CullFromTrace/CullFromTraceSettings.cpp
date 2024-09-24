// Fill out your copyright notice in the Description page of Project Settings.


#include "CullFromTraceSettings.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "Data/PCGPointData.h"

#define LOCTEXT_NAMESPACE "PCGWorldQuery_SphereTrace"
namespace PCGCullSettings
{
	static const FName PointsLabel = TEXT("Points");
	static const FName RemovedPointsLabel = TEXT("Removed Points");
    
}


UCullFromTraceSettings::UCullFromTraceSettings()
{
	
}

TArray<FPCGPinProperties> UCullFromTraceSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;

	FPCGPinProperties SourcePin;
	SourcePin.Label = FName("Source");
	SourcePin.AllowedTypes = EPCGDataType::Point;
	SourcePin.PinStatus = EPCGPinStatus::Required;
	
	Properties.Emplace(SourcePin);
	return Properties;
}

TArray<FPCGPinProperties> UCullFromTraceSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;

	Properties.Emplace(PCGCullSettings::PointsLabel, EPCGDataType::Point);
	Properties.Emplace(PCGCullSettings::RemovedPointsLabel, EPCGDataType::Point);

	return Properties;
}

FPCGElementPtr UCullFromTraceSettings::CreateElement() const
{
	return MakeShared<FPCGCullPointsElement>();
}

/// ---------------------------------------------------------
/// POINT LOOP
bool FPCGCullPointsElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGFilterBySeedValueElement::Execute);

	check(Context);

	const UCullFromTraceSettings* Settings = Context->GetInputSettings<UCullFromTraceSettings>();
	check(Settings);
	
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

		UPCGPointData* CulledPointsData = NewObject<UPCGPointData>();
		CulledPointsData->InitializeFromData(PointData);
		TArray<FPCGPoint>& CulledPoints = CulledPointsData->GetMutablePoints();
		
		TArray<FHitResult> OutHits;
        
		for (int i =0; i < InPoints.Num(); i++)
		{
			FPCGPoint AlteredPoint = InPoints[i]; 
			FVector TraceLocation = InPoints[i].Transform.GetLocation();
			if (Trace(World, TraceLocation, TraceLocation, Settings->ChannelsToTrace, Settings->SphereRadius, OutHits))
			{
				for (const auto& Hit : OutHits)
				{
					const auto* HitActor = Hit.GetActor();
					const auto* HitComponent = Hit.GetComponent();
					
					if (IsValid(HitActor))
					{
						if (ContainsAny(HitActor->Tags, Settings->ActorTagsToCull))
						{
							CulledPoints.Add(AlteredPoint);
							continue;
						}
						
						if (IsValid(HitComponent) && ContainsAny(HitComponent->ComponentTags, Settings->ComponentTagsToCull))
						{
							CulledPoints.Add(AlteredPoint);
							continue;
						}
					}

					if(Hit.Normal.Equals(FVector::UpVector)) continue;
					ChosenPoints.Add(AlteredPoint);
				}
			}
			else
			{
				ChosenPoints.Add(AlteredPoint);
			}
		}
        
        
		// Output all in output collection
		FPCGTaggedData& ChosenTaggedData = Outputs.Add_GetRef(Input);
		ChosenTaggedData.Data = ChosenPointsData;
		ChosenTaggedData.Pin = PCGCullSettings::PointsLabel;

		FPCGTaggedData& CulledTaggedData = Outputs.Add_GetRef(Input);
		CulledTaggedData.Data = CulledPointsData;
		CulledTaggedData.Pin = PCGCullSettings::RemovedPointsLabel;

		
	}

	return true;
}

bool FPCGCullPointsElement::Trace(const UWorld* World, const FVector& Start, const FVector& End, const TArray<TEnumAsByte<ECollisionChannel>>& Types, const float& Radius, TArray<FHitResult>& OutHits) const
{
	if (!World) return false;

	FCollisionObjectQueryParams Params;
	for (const auto& Type : Types)
	{
		Params.AddObjectTypesToQuery(Type);
	}
    
	return World->SweepMultiByObjectType(OutHits, Start, End, FQuat::Identity, Params, FCollisionShape::MakeSphere(Radius));
}

bool FPCGCullPointsElement::ContainsAny(const TArray<FName>& Array, const TArray<FName>& Compare) const
{
	for (const auto& Element : Compare)
	{
		if (Array.Contains(Element))
		{
			return true;
		}
	}

	return false;
}


#undef LOCTEXT_NAMESPACE
