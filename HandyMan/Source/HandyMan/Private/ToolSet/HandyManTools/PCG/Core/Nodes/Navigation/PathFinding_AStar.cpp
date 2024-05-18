// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManTools/PCG/Core/Nodes/Navigation/PathFinding_AStar.h"

#include "PCGContext.h"
#include "Data/PCGPointData.h"
#include "ToolSet/Algorithms/AStarPathFinding/PCGPathfindHelper.h"

#define LOCTEXT_NAMESPACE "PCGGroupRandomPairsSettings"

namespace PCGPathFindingSettings
{
    static const FName FinalPointsLabel = TEXT("Path");
    

    static const FName StartingPointLabel = TEXT("StartPoint");
    static const FName EndingPointLabel = TEXT("EndPoint");
    static const FName PathPointsLabel = TEXT("PathPoints");
    
}

UPathFinding_AStarSettings::UPathFinding_AStarSettings()
{
    bUseSeed = true;
}

FName UPathFinding_AStarSettings::AdditionalTaskName() const
{
    return NAME_None;
}

TArray<FPCGPinProperties> UPathFinding_AStarSettings::InputPinProperties() const
{
    TArray<FPCGPinProperties> Properties;

    Properties.Emplace(PCGPathFindingSettings::StartingPointLabel, EPCGDataType::Point);
    Properties.Emplace(PCGPathFindingSettings::EndingPointLabel, EPCGDataType::Point);
    Properties.Emplace(PCGPathFindingSettings::PathPointsLabel, EPCGDataType::Point);

    return Properties;
}

TArray<FPCGPinProperties> UPathFinding_AStarSettings::OutputPinProperties() const
{
    TArray<FPCGPinProperties> Properties;

    Properties.Emplace(PCGPathFindingSettings::FinalPointsLabel, EPCGDataType::Point);
    
    return Properties;
}

FPCGElementPtr UPathFinding_AStarSettings::CreateElement() const
{
    return MakeShared<FPCGAStarPathfindingElement>();
}

bool FPCGAStarPathfindingElement::ExecuteInternal(FPCGContext* Context) const
{
   
    TRACE_CPUPROFILER_EVENT_SCOPE(FPCGAStarPathfindingElement::Execute);

    check(Context);

    const UPathFinding_AStarSettings* Settings = Context->GetInputSettings<UPathFinding_AStarSettings>();
    check(Settings);
    

    TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

    const FPCGPoint& StartingPoint = Cast<UPCGPointData>(Context->InputData.GetInputsByPin(PCGPathFindingSettings::StartingPointLabel)[0].Data)->GetPoint(0);
    const FPCGPoint& EndingPoint = Cast<UPCGPointData>(Context->InputData.GetInputsByPin(PCGPathFindingSettings::EndingPointLabel)[0].Data)->GetPoint(0);
    const TArray<FPCGPoint>& PathPoints = Cast<UPCGPointData>(Context->InputData.GetInputsByPin(PCGPathFindingSettings::PathPointsLabel)[0].Data)->GetPoints();

    const TArray<FPCGPoint>& NewPoints = UPCGPathfindHelper::FindPath(StartingPoint, EndingPoint, PathPoints);

    UPCGPointData* ChosenPointsData = NewObject<UPCGPointData>();
    ChosenPointsData->InitializeFromData(Cast<UPCGPointData>(Context->InputData.GetInputs()[2].Data));
    ChosenPointsData->SetPoints(NewPoints);
   
    // Output all in output collection
    FPCGTaggedData& ChosenTaggedData = Outputs.Add_GetRef(Context->InputData.GetInputs()[1]);
    ChosenTaggedData.Data = ChosenPointsData;
    ChosenTaggedData.Pin = PCGPathFindingSettings::FinalPointsLabel;

    return true;
}

#undef LOCTEXT_NAMESPACE