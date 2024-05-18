// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/PCG/Core/Nodes/Filter/FilterBySeedValue.h"

#include "PCGContext.h"
#include "Data/PCGPointData.h"

#define LOCTEXT_NAMESPACE "PCGFilterBySeedValueSettings"

namespace PCGFilterBySeedValueSettings
{
    static const FName PairedPointsLabel = TEXT("MatchingPoint");
    
}

UFilterBySeedValueSettings::UFilterBySeedValueSettings()
{
    bUseSeed = true;
}

FName UFilterBySeedValueSettings::AdditionalTaskName() const
{
    return NAME_None;
}

TArray<FPCGPinProperties> UFilterBySeedValueSettings::OutputPinProperties() const
{
    TArray<FPCGPinProperties> Properties;

    Properties.Emplace(PCGFilterBySeedValueSettings::PairedPointsLabel, EPCGDataType::Point);

    return Properties;
}

FPCGElementPtr UFilterBySeedValueSettings::CreateElement() const
{
    return MakeShared<FPCGFilterBySeedValueElement>();
}

bool FPCGFilterBySeedValueElement::ExecuteInternal(FPCGContext* Context) const
{
   
    TRACE_CPUPROFILER_EVENT_SCOPE(FPCGFilterBySeedValueElement::Execute);

    check(Context);

    const UFilterBySeedValueSettings* Settings = Context->GetInputSettings<UFilterBySeedValueSettings>();
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
        
        for (int i =0; i < InPoints.Num(); i++)
        {

            if (InPoints[i].Seed == Settings->SeedToCompare)
            {
                FPCGPoint AlteredPoint = InPoints[i];
                ChosenPoints.Add(AlteredPoint);
               break;
            }
        }
        
        
        // Output all in output collection
        FPCGTaggedData& ChosenTaggedData = Outputs.Add_GetRef(Input);
        ChosenTaggedData.Data = ChosenPointsData;
        ChosenTaggedData.Pin = PCGFilterBySeedValueSettings::PairedPointsLabel;
        

    }

    return true;
}

#undef LOCTEXT_NAMESPACE