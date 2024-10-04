// Fill out your copyright notice in the Description page of Project Settings.


#include "HandyManMeshSurfaceTool.h"
#include "InteractiveToolManager.h"
#include "ToolTargets/PrimitiveComponentToolTarget.h"

UHandyManMeshSurfaceToolBuilder::UHandyManMeshSurfaceToolBuilder(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	AcceptedClasses.Add(UPrimitiveComponentBackedTarget::StaticClass());
}

void UHandyManMeshSurfaceToolBuilder::SetupTool(const FToolBuilderState& SceneState, UInteractiveTool* Tool) const
{
	Super::SetupTool(SceneState, Tool);

	
}

/*void UHandyManMeshSurfaceToolBuilder::OnSetupTool_Implementation(UScriptableInteractiveTool* Tool, const TArray<AActor*>& SelectedActors, const TArray<UActorComponent*>& SelectedComponents) const
{
	if (UHandyManMeshSurfaceTool* SurfaceTool = Cast<UHandyManMeshSurfaceTool>(Tool))
	{
		SurfaceTool->SetStylusAPI(StylusAPI);
	}
}*/


///----------------------------------------------------------------------
/// TOOL 

void UHandyManMeshSurfaceTool::SetStylusAPI(IHandyManStylusStateProviderAPI* StylusAPIIn)
{
	this->StylusAPI = StylusAPIIn;
}

bool UHandyManMeshSurfaceTool::HitTest(const FRay& Ray, FHitResult& OutHit)
{
	return Cast<IPrimitiveComponentBackedTarget>(Targets[0])->HitTestComponent(Ray, OutHit);
}

bool UHandyManMeshSurfaceTool::SupportsWorldSpaceFocusBox()
{
	return Targets[0] && Cast<UPrimitiveComponentToolTarget>(Targets[0]) != nullptr;
}


FBox UHandyManMeshSurfaceTool::GetWorldSpaceFocusBox()
{
	if (Targets[0])
	{
		UPrimitiveComponentToolTarget* PrimTarget = Cast<UPrimitiveComponentToolTarget>(Targets[0]);
		if (PrimTarget)
		{
			UPrimitiveComponent* Component = PrimTarget->GetOwnerComponent();
			if (Component)
			{
				return Component->Bounds.GetBox();
			}
		}
	}
	return FBox();
}


bool UHandyManMeshSurfaceTool::SupportsWorldSpaceFocusPoint()
{
	return Targets[0] && Cast<UPrimitiveComponentToolTarget>(Targets[0]) != nullptr;
}

float UHandyManMeshSurfaceTool::GetCurrentDevicePressure() const
{
	return (StylusAPI != nullptr) ? FMath::Clamp(StylusAPI->GetCurrentPressure(), 0.0f, 1.0f) : 1.0f;
}

bool UHandyManMeshSurfaceTool::GetWorldSpaceFocusPoint(const FRay& WorldRay, FVector& PointOut)
{
	FHitResult HitResult;
	if (HitTest(WorldRay, HitResult))
	{
		PointOut = HitResult.ImpactPoint;
		return true;
	}
	return false;
}

void UHandyManMeshSurfaceTool::OnBeginDrag(const FRay& Ray)
{

}


void UHandyManMeshSurfaceTool::OnUpdateDrag(const FRay& Ray)
{
	FHitResult OutHit;
	if ( HitTest(Ray, OutHit) ) 
	{
		GetToolManager()->DisplayMessage( 
			FText::Format(NSLOCTEXT("UHandyManMeshSurfaceTool", "OnUpdateDragMessage", "UHandyManMeshSurfaceTool::OnUpdateDrag: Hit triangle index {0} at ray distance {1}"),
				FText::AsNumber(OutHit.FaceIndex), FText::AsNumber(OutHit.Distance)),
			EToolMessageLevel::Internal);
	}
}

void UHandyManMeshSurfaceTool::OnEndDrag(const FRay& Ray)
{
	//GetToolManager()->DisplayMessage(TEXT("UHandyManMeshSurfaceTool::OnEndDrag!"), EToolMessageLevel::Internal);
}

FInputRayHit UHandyManMeshSurfaceTool::OnHoverHitTest_Implementation(FInputDeviceRay HoverPos, const FScriptableToolModifierStates& Modifiers)
{
	LastWorldRay = HoverPos.WorldRay;
	FHitResult OutHit;
	if (HitTest(HoverPos.WorldRay, OutHit))
	{
		return FInputRayHit(OutHit.Distance);
	}
	return FInputRayHit();
}

FInputRayHit UHandyManMeshSurfaceTool::TestIfCanBeginClickDrag_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	FHitResult OutHit;
	if (HitTest(ClickPos.WorldRay, OutHit))
	{
		return FInputRayHit(OutHit.Distance);
	}
	return FInputRayHit();
}

void UHandyManMeshSurfaceTool::OnDragBegin_Implementation(FInputDeviceRay StartPosition, const FScriptableToolModifierStates& Modifiers)
{
	LastWorldRay = StartPosition.WorldRay;
	OnBeginDrag(StartPosition.WorldRay);
}

void UHandyManMeshSurfaceTool::OnDragUpdatePosition_Implementation(FInputDeviceRay NewPosition,
	const FScriptableToolModifierStates& Modifiers)
{
	LastWorldRay = NewPosition.WorldRay;
	OnUpdateDrag(NewPosition.WorldRay);
}

void UHandyManMeshSurfaceTool::OnDragEnd_Implementation(FInputDeviceRay EndPosition, const FScriptableToolModifierStates& Modifiers)
{
	LastWorldRay = EndPosition.WorldRay;
	OnEndDrag(EndPosition.WorldRay);
}

void UHandyManMeshSurfaceTool::OnDragSequenceCancelled_Implementation()
{
	OnCancelDrag();
}
