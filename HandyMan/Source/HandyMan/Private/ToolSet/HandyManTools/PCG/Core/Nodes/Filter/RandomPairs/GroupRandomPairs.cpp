// Fill out your copyright notice in the Description page of Project Settings.


#include "GroupRandomPairs.h"
#include "PCGContext.h"
#include "Data/PCGPointData.h"
#include "Metadata/PCGMetadataAccessor.h"


#define LOCTEXT_NAMESPACE "PCGGroupRandomPairsSettings"

namespace PCGRandomGroupPairSettings
{
    static const FName PairedPointsLabel = TEXT("PointsWithPair");
    static const FName UsedPointsLabel = TEXT("PairPoints");
    static const FName ExcessPointsLabel = TEXT("UnpairedPoints");
    
}

UGroupRandomPairsSettings::UGroupRandomPairsSettings()
{
    bUseSeed = true;
}

FName UGroupRandomPairsSettings::AdditionalTaskName() const
{
    return NAME_None;
}

TArray<FPCGPinProperties> UGroupRandomPairsSettings::OutputPinProperties() const
{
    TArray<FPCGPinProperties> Properties;

    Properties.Emplace(PCGRandomGroupPairSettings::PairedPointsLabel, EPCGDataType::Point);
    Properties.Emplace(PCGRandomGroupPairSettings::UsedPointsLabel, EPCGDataType::Point);
    Properties.Emplace(PCGRandomGroupPairSettings::ExcessPointsLabel, EPCGDataType::Point);

    return Properties;
}

FPCGElementPtr UGroupRandomPairsSettings::CreateElement() const
{
    return MakeShared<FPCGGroupRandomPairsElement>();
}

bool FPCGGroupRandomPairsElement::ExecuteInternal(FPCGContext* Context) const
{
   
    TRACE_CPUPROFILER_EVENT_SCOPE(FPCGGroupRandomPairsElement::Execute);

    check(Context);

    const UGroupRandomPairsSettings* Settings = Context->GetInputSettings<UGroupRandomPairsSettings>();
    check(Settings);

    TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

    for (const FPCGTaggedData& Input : Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel))
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

        UPCGPointData* DiscardedPointsData = NewObject<UPCGPointData>();
        DiscardedPointsData->InitializeFromData(PointData);
        TArray<FPCGPoint>& DiscardedPoints = DiscardedPointsData->GetMutablePoints();

        UPCGPointData* UsedPointsData = NewObject<UPCGPointData>();
        UsedPointsData->InitializeFromData(PointData);
        TArray<FPCGPoint>& UsedPoints = UsedPointsData->GetMutablePoints();

        FRandomStream RandomStream(Context->GetSeed());
        TArray<int64> ChosenPointSeedArray;
        
        for (int i =0; i < InPoints.Num(); i++)
        {

            // Check the attribute value to filter the points. TODO: Make this more generic to handle all types of attributes
            const FPCGMetadataAttribute<bool>* StartingPointAttribute = static_cast<FPCGMetadataAttribute<bool>*>((PointData->Metadata->GetMutableAttribute(Settings->StartPointAttributeName)));
            if (StartingPointAttribute && StartingPointAttribute->GetValueFromItemKey(InPoints[i].MetadataEntry))
            {
                FPCGPoint AlteredPoint = InPoints[i];
                int64 Selection = -1;
                TArray<FPCGPoint> ItemsToChooseFrom;
                for (auto Point : InPoints)
                {
                    const FPCGMetadataAttribute<bool>* EndingPointAttribute = static_cast<FPCGMetadataAttribute<bool>*>((PointData->Metadata->GetMutableAttribute(Settings->ExcessPointAttributeName)));

                    const bool bIsAcceptableItem = !ChosenPointSeedArray.Contains(Point.Seed) || ChosenPointSeedArray.IsEmpty();
                    const bool bIsNotSameItem = Point.Seed != InPoints[i].Seed;
                    if (EndingPointAttribute && EndingPointAttribute->GetValueFromItemKey(Point.MetadataEntry) && bIsAcceptableItem && bIsNotSameItem)
                    {
                       ItemsToChooseFrom.Add(Point);
                    }
                }

                FPCGPoint PointPair = ItemsToChooseFrom.Num() > 0 ? ItemsToChooseFrom[RandomStream.RandRange(0, ItemsToChooseFrom.Num() - 1)] : FPCGPoint();
                Selection = PointPair.Seed;
                ChosenPointSeedArray.Add(PointPair.Seed);
                FPCGMetadataAttribute<int64>* Attribute = ChosenPointsData->Metadata->FindOrCreateAttribute<int64>(Settings->AttributeNameToAdjust, -1, true, true);
                Attribute->SetValue(AlteredPoint.MetadataEntry, Selection);
                ChosenPoints.Add(AlteredPoint);
                UsedPoints.Add(PointPair);
                continue;
                
            }

            const FPCGMetadataAttribute<bool>* ExcessPointsAttribute = static_cast<FPCGMetadataAttribute<bool>*>((PointData->Metadata->GetMutableAttribute(Settings->ExcessPointAttributeName)));
            if (ExcessPointsAttribute && ExcessPointsAttribute->GetValueFromItemKey(InPoints[i].MetadataEntry))
            {
                DiscardedPoints.Add(InPoints[i]);
            }
        }
        
        
        // Output all in output collection
        FPCGTaggedData& ChosenTaggedData = Outputs.Add_GetRef(Input);
        ChosenTaggedData.Data = ChosenPointsData;
        ChosenTaggedData.Pin = PCGRandomGroupPairSettings::PairedPointsLabel;

        FPCGTaggedData& DiscardedTaggedData = Outputs.Add_GetRef(Input);
        DiscardedTaggedData.Data = DiscardedPointsData;
        DiscardedTaggedData.Pin = PCGRandomGroupPairSettings::ExcessPointsLabel;

        // Output all in output collection
        FPCGTaggedData& UsedTaggedData = Outputs.Add_GetRef(Input);
        UsedTaggedData.Data = UsedPointsData;
        UsedTaggedData.Pin = PCGRandomGroupPairSettings::UsedPointsLabel;

    }

    return true;
}

#undef LOCTEXT_NAMESPACE