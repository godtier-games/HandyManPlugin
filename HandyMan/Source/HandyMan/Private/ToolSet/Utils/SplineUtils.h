#pragma once

#include <CoreMinimal.h>
#include "Containers/StaticArray.h"
#include "Serialization/Archive.h"
#include <UObject/UE5MainStreamObjectVersion.h>
#include "VectorTypes.h"
#include "InteractiveToolChange.h"
#include "ToolContextInterfaces.h"
#include "Components/SplineComponent.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "Math/NumericLimits.h"




// Helper class for making undo/redo transactions, to avoid friending all the variations.
class FSplineToolChange : public FToolCommandChange
{
public:
	// These pass the working spline to the overloads below
	virtual void Apply(UObject* Object) override {};
	virtual void Revert(UObject* Object) override {};

protected:
	virtual void Apply(USplineComponent& Spline) = 0;
	virtual void Revert(USplineComponent& Spline) = 0;
};

#define LOCTEXT_NAMESPACE "SplineTool"

namespace HandyMan::SplineUtils
{
	inline FText AddPointTransactionName = LOCTEXT("AddPointTransactionName", "Add Point");

	inline USplineComponent* CreateNewSplineInActor(AActor* Actor, bool bTransact = false, bool bSetAsRoot = false)
	{
		if (!ensure(Actor))
		{
			return nullptr;
		}

		if (bTransact)
		{
			Actor->Modify();
		}

		FName NewComponentName = *FComponentEditorUtils::GenerateValidVariableName(USplineComponent::StaticClass(), Actor);
		// Note that the RF_Transactional is important here for the spline to undo/redo properly in the future
		USplineComponent* Spline = NewObject<USplineComponent>(Actor, USplineComponent::StaticClass(), NewComponentName, 
			bTransact ? RF_Transactional : RF_NoFlags);

		if (bSetAsRoot)
		{
			Actor->SetRootComponent(Spline);
		}
		else
		{
			Spline->SetupAttachment(Actor->GetRootComponent());
		}

		Spline->OnComponentCreated();
		Actor->AddInstanceComponent(Spline);
		Spline->RegisterComponent();
		Spline->ResetRelativeTransform();
		Actor->PostEditChange();

		return Spline;
	};

	inline USplineComponent* GetOrCreateTargetSpline(AActor* Actor, int32 TargetIndex, bool bTransact = false)
	{
		if (TargetIndex >= 0)
		{
			TInlineComponentArray<USplineComponent*> SplineComponents;
			Actor->GetComponents<USplineComponent>(SplineComponents);
			if (TargetIndex < SplineComponents.Num())
			{
				return SplineComponents[TargetIndex];
			}
		}

		// If we got to here, we didn't have an existing target at that index. Create one.
		return CreateNewSplineInActor(Actor, bTransact);
	}

	inline void CopySplineToSpline(const USplineComponent& Source, USplineComponent& Destination, bool bTransact = false)
	{
		if (bTransact)
		{
			Destination.Modify();
		}

		Destination.ClearSplinePoints();
		Destination.bSplineHasBeenEdited = true;

		// We iterate here (rather than just copying over the SplineCurves data) so that we can transform
		// the data properly into the coordinate space of the target component.
		int32 NumSplinePoints = Source.GetNumberOfSplinePoints();
		for (int32 i = 0; i < NumSplinePoints; ++i)
		{
			Destination.AddSplinePoint(Source.GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World),
				ESplineCoordinateSpace::World, false);
			Destination.SetUpVectorAtSplinePoint(i, Source.GetUpVectorAtSplinePoint(i, ESplineCoordinateSpace::World),
				ESplineCoordinateSpace::World, false);
			Destination.SetTangentsAtSplinePoint(i,
				Source.GetArriveTangentAtSplinePoint(i, ESplineCoordinateSpace::World),
				Source.GetLeaveTangentAtSplinePoint(i, ESplineCoordinateSpace::World),
				ESplineCoordinateSpace::World, false);
			Destination.SetSplinePointType(i, Source.GetSplinePointType(i), false);
		}

		Destination.SetClosedLoop(Source.IsClosedLoop());

		Destination.UpdateSpline();
	};

	// Gives the scale used for tangent visualization (and which therefore needs to be used in raycasting the handles)
	inline float GetTangentScale()
	{
		return GetDefault<ULevelEditorViewportSettings>()->SplineTangentScale;
	}

	// Might be useful to have in SplineUtil, but uncertain what the API should be (should it be part of
	// DrawSpline? Should there be options for selection color?). Also potentially messier to match the tangent
	// scale with the UI interaction..
	inline void DrawTangent(const USplineComponent& SplineComp, int32 PointIndex, IToolsContextRenderAPI& RenderAPI)
	{
		if (!ensure(PointIndex >= 0 && PointIndex < SplineComp.GetNumberOfSplinePoints()))
		{
			return;
		}

		FPrimitiveDrawInterface* PDI = RenderAPI.GetPrimitiveDrawInterface();

		const float TangentScale = GetDefault<ULevelEditorViewportSettings>()->SplineTangentScale;
		const float TangentHandleSize = 8.0f + GetDefault<ULevelEditorViewportSettings>()->SplineTangentHandleSizeAdjustment;

		const FVector Location = SplineComp.GetLocationAtSplinePoint(PointIndex, ESplineCoordinateSpace::World);
		const FVector LeaveTangent = SplineComp.GetLeaveTangentAtSplinePoint(PointIndex, ESplineCoordinateSpace::World) * TangentScale;
		const FVector ArriveTangent = SplineComp.bAllowDiscontinuousSpline ?
			SplineComp.GetArriveTangentAtSplinePoint(PointIndex, ESplineCoordinateSpace::World) * TangentScale : LeaveTangent;

		FColor Color = FColor::White;

		PDI->DrawLine(Location, Location - ArriveTangent, Color, SDPG_Foreground);
		PDI->DrawLine(Location, Location + LeaveTangent, Color, SDPG_Foreground);

		PDI->DrawPoint(Location + LeaveTangent, Color, TangentHandleSize, SDPG_Foreground);
		PDI->DrawPoint(Location - ArriveTangent, Color, TangentHandleSize, SDPG_Foreground);
	}

	// Undoes a point addition with an auto tangent
	class FSimplePointInsertionChange : public FSplineToolChange
	{
	public:
		virtual ~FSimplePointInsertionChange() = default;

		FSimplePointInsertionChange(const FVector3d& HitLocationIn, const FVector3d& UpVectorIn)
			: HitLocation(HitLocationIn)
			, UpVector(UpVectorIn)
		{
		}

		virtual void Apply(USplineComponent& Spline) override
		{
			Spline.AddSplinePoint(HitLocation, ESplineCoordinateSpace::World, false);
			int32 PointIndex = Spline.GetNumberOfSplinePoints() - 1;
			Spline.SetUpVectorAtSplinePoint(PointIndex, UpVector, ESplineCoordinateSpace::World, true);
		}
		virtual void Revert(USplineComponent& Spline) override
		{
			if (ensure(Spline.GetNumberOfSplinePoints() > 0))
			{
				Spline.RemoveSplinePoint(Spline.GetNumberOfSplinePoints() - 1, true);
			}
		}
		virtual FString ToString() const override
		{
			return TEXT("FSimplePointInsertionChange");
		}

	protected:
		FVector3d HitLocation;
		FVector3d UpVector;
	};

	// Undoes a point addition with an explicit tangent
	class FTangentPointInsertionChange : public FSplineToolChange
	{
	public:
		FTangentPointInsertionChange(const FVector3d& HitLocationIn, const FVector3d& UpVectorIn, const FVector3d& TangentIn)
			: HitLocation(HitLocationIn)
			, UpVector(UpVectorIn)
			, Tangent(TangentIn)
		{
		}

		virtual void Apply(USplineComponent& Spline) override
		{
			Spline.AddSplinePoint(HitLocation, ESplineCoordinateSpace::World, false);
			int32 PointIndex = Spline.GetNumberOfSplinePoints() - 1;
			Spline.SetUpVectorAtSplinePoint(PointIndex, UpVector, ESplineCoordinateSpace::World, false);
			Spline.SetTangentAtSplinePoint(PointIndex, Tangent, ESplineCoordinateSpace::World, true);
		}
		virtual void Revert(USplineComponent& Spline) override
		{
			if (ensure(Spline.GetNumberOfSplinePoints() > 0))
			{
				Spline.RemoveSplinePoint(Spline.GetNumberOfSplinePoints() - 1, true);
			}
		}
		virtual FString ToString() const override
		{
			return TEXT("FTangentPointInsertionChange");
		}

	protected:
		FVector3d HitLocation;
		FVector3d UpVector;
		FVector3d Tangent;
	};

	// Undoes a free draw stroke (multiple points at once)
	class FStrokeInsertionChange : public FSplineToolChange
	{
	public:
		FStrokeInsertionChange(const TArray<FVector3d>& HitLocationsIn, const TArray<FVector3d>& UpVectorsIn)
			: HitLocations(HitLocationsIn)
			, UpVectors(UpVectorsIn)
		{
			if (!ensure(HitLocations.Num() == UpVectors.Num()))
			{
				int32 Num = FMath::Min(HitLocations.Num(), UpVectors.Num());
				HitLocations.SetNum(Num);
				UpVectors.SetNum(Num);
			}
		}

		virtual void Apply(USplineComponent& Spline) override
		{
			for (int32 i = 0; i < HitLocations.Num(); ++i)
			{
				Spline.AddSplinePoint(HitLocations[i], ESplineCoordinateSpace::World, false);
				int32 PointIndex = Spline.GetNumberOfSplinePoints() - 1;
				Spline.SetUpVectorAtSplinePoint(PointIndex, UpVectors[i], ESplineCoordinateSpace::World, false);
			}
			Spline.UpdateSpline();
		}
		virtual void Revert(USplineComponent& Spline) override
		{
			for (int32 i = 0; i < HitLocations.Num(); ++i)
			{
				if (!ensure(Spline.GetNumberOfSplinePoints() > 0))
				{
					break;
				}
				Spline.RemoveSplinePoint(Spline.GetNumberOfSplinePoints() - 1, false);
			}
			Spline.UpdateSpline();
		}
		virtual FString ToString() const override
		{
			return TEXT("FStrokeInsertionChange");
		}

	protected:
		TArray<FVector3d> HitLocations;
		TArray<FVector3d> UpVectors;
	};
}

#undef LOCTEXT_NAMESPACE