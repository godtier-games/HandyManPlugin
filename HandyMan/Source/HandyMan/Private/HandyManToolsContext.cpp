// Fill out your copyright notice in the Description page of Project Settings.


#include "HandyManToolsContext.h"

#include "HandyManPhysicsInterface.h"

void UHandyManToolsContext::DeactivateAllActiveTools(EToolShutdownType ShutdownType)
{
	if (!ensureMsgf(ToolManager, TEXT("Trying to deactivate all tools on a ToolManager that has already been deleted.")))
	{
		return;
	}
	auto DeactivateTool = [this, ShutdownType](EToolSide WhichSide) {
		
		if (ToolManager->HasActiveTool(WhichSide) && !ToolManager->GetActiveTool(WhichSide)->Implements<UHandyManPhysicsInterface>())
		{
			EToolShutdownType ShutdownTypeToUse = ShutdownType;

			// We do not allow passing an "accept" shutdown type if the tool says that it cannot accept.
			// For complete-style tools, which can never accept, it should theoretically not matter what
			// we pass because we would like them to respond the same way to all three shutdown types.
			// However, we currently have one complete-style tool that responds differently to cancel, and we
			// can't change that in a hotfix. So, we pass "complete" to complete-style tools.
			if (ShutdownType != EToolShutdownType::Cancel)
			{
				if (!ToolManager->GetActiveTool(WhichSide)->HasAccept())
				{
					ShutdownTypeToUse = EToolShutdownType::Completed;
				}
				else if (ToolManager->CanAcceptActiveTool(WhichSide))
				{
					ShutdownTypeToUse = EToolShutdownType::Accept;
				}
				else
				{
					ShutdownTypeToUse = EToolShutdownType::Cancel;
				}
			}

			ToolManager->DeactivateTool(WhichSide, ShutdownTypeToUse);
		}
	};
	
	DeactivateTool(EToolSide::Left);
	DeactivateTool(EToolSide::Right);

	RestoreEditorState();
}
