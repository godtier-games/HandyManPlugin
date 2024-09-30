// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/ToolBuilders/HandyManToolWithTargetBuilder.h"

#include "ScriptableInteractiveTool.h"
#include "ToolTargetManager.h"


UHandyManToolWithTargetBuilder::UHandyManToolWithTargetBuilder(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UHandyManToolWithTargetBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	const bool bIsBlueprintAccepted = Super::CanBuildTool(SceneState);
	int MatchingTargetCount = SceneState.TargetManager->CountSelectedAndTargetable(SceneState, GetRequirements());
	return MatchingTargetCount >= MinRequiredMatches && (MatchingTargetCount <= MatchLimit || MatchLimit < 0) && bIsBlueprintAccepted;
}

void UHandyManToolWithTargetBuilder::SetupTool(const FToolBuilderState& SceneState, UInteractiveTool* Tool) const
{
	UScriptableInteractiveTool* NewTool = Cast<UScriptableInteractiveTool>(Tool);
	TArray<TObjectPtr<UToolTarget>> Targets = SceneState.TargetManager->BuildAllSelectedTargetable(SceneState, GetRequirements());
	const int NumTargets = Targets.Num();
	Targets.SetNum(FMath::Min(NumTargets, AcceptedClasses.Num()));
	NewTool->SetTargets(Targets);
	OnSetupTool(NewTool, SceneState.SelectedActors, SceneState.SelectedComponents);
}

FToolTargetTypeRequirements UHandyManToolWithTargetBuilder::GetRequirements() const
{
	FToolTargetTypeRequirements Requirements;
	for (UClass* ClassPtr : AcceptedClasses)
	{
		Requirements.Add(ClassPtr);
	}

	return Requirements;
}
