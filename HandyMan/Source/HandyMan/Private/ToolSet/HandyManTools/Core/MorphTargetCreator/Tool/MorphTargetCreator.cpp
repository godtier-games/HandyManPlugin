// Fill out your copyright notice in the Description page of Project Settings.


#include "MorphTargetCreator.h"
#include "Components/DynamicMeshComponent.h"
#include "InteractiveToolManager.h"
#include "ModelingToolTargetUtil.h"
#include "PreviewMesh.h"
#include "Selection.h"
#include "ToolBuilderUtil.h"
#include "ToolDataVisualizer.h"
#include "ToolSetupUtil.h"
#include "ToolTargetManager.h"
#include "UnrealEdGlobals.h"
#include "Animation/SkeletalMeshActor.h"
#include "AssetUtils/Texture2DUtil.h"
#include "DynamicMesh/MeshIndexUtil.h"
#include "Editor/UnrealEdEngine.h"
#include "Generators/RectangleMeshGenerator.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "Sculpting/MeshSculptUtil.h"
#include "Selections/MeshConnectedComponents.h"
#include "TargetInterfaces/DynamicMeshCommitter.h"
#include "TargetInterfaces/DynamicMeshProvider.h"
#include "TargetInterfaces/MaterialProvider.h"
#include "TargetInterfaces/MeshTargetInterfaceTypes.h"
#include "ToolSet/Core/HandyManSubsystem.h"
#include "ToolSet/HandyManTools/Core/MorphTargetCreator/Actor/MorphTargetCreatorProxyActor.h"
#include "ToolSet/HandyManTools/Core/SculptTool/DataTypes/HandyManSculptingTypes.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Gizmos/HandyManBrushStampIndicator.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Operators/Inflate/HandyManInflateBrushOps.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Operators/Kelvin/HandyManKelvinletBrushOp.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Operators/MeshSculpt/HandyManMeshSculptBrushOps.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Operators/Move/HandyManMoveBrushOps.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Operators/Pinch/HandyManPinchBrushOps.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Operators/Planar/HandyManPlaneBrushOps.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Operators/Smoothing/HandyManMeshSmoothingBrushOps.h"
#include "ToolSet/HandyManTools/Core/SculptTool/Utils/HandyManSculptUtil.h"
#include "ToolTargets/PrimitiveComponentToolTarget.h"

using namespace UE::Geometry;

namespace
{
	// probably should be something defined for the whole tool framework...
#if WITH_EDITOR
	static EAsyncExecution VertexSculptToolAsyncExecTarget = EAsyncExecution::LargeThreadPool;
#else
	static EAsyncExecution VertexSculptToolAsyncExecTarget = EAsyncExecution::ThreadPool;
#endif
}



#define LOCTEXT_NAMESPACE "MorphTargetCreator"

#if WITH_EDITOR

TArray<FString> UMorphTargetProperties::GetMorphTargetNames() const
{
	TArray<FString> result;
	if (TargetMesh)
	{
		for (const auto& Morph : TargetMesh->GetMorphTargets())
		{
			if(Morph.GetName().IsEmpty()) continue;
			result.Add(Morph.GetName());
		}
	}
	
	return result;
}

  void UMorphTargetProperties::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	UMorphTargetCreator* ParentToolPtr = Cast<UMorphTargetCreator>(ParentTool.Get());
	if (!ParentToolPtr) return;

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, TargetMesh))
	{
		if (IsValid(TargetMesh))
		{
			GEditor->MoveViewportCamerasToComponent(ParentToolPtr->DynamicMeshComponent, true);
		}
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MorphTargets))
	{
		
		// If we have removed a mesh then we need to tell our tool to remove that dynamic mesh
		if (ParentToolPtr->bHasToolStarted)
		{
			if (MorphTargets.Num() < ParentToolPtr->GetMorphTargetMeshMap().Num())
			{
				// Find the missing morph
			
				TArray<FName> CachedMorphs;
				Meshes.GetKeys(CachedMorphs);

				FName MorphToRemove = NAME_None;
				for (int i = CachedMorphs.Num() - 1; i >= 0; --i)
				{
					if(MorphTargets.Contains(CachedMorphs[i])) continue;
					MorphToRemove = CachedMorphs[i];
					break;
				}

				// Tell the tool to remove the missing morph
				ParentToolPtr->RemoveMorphTargetMesh(MorphToRemove);
				Meshes = ParentToolPtr->GetMorphTargetMeshMap();
				return;
			}
		
			// Tell our tool to generate a new dynamic mesh and store that in the meshes
			if (!MorphTargets.Last().IsEqual(NAME_None))
			{
				ParentToolPtr->CreateMorphTargetMesh(MorphTargets.Last());
				Meshes = ParentToolPtr->GetMorphTargetMeshMap();
				ParentToolPtr->TriggerToolStartUp();
			}
		}
		else
		{
			FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
			NSLOCTEXT("UBuildingGeneratorTool", "ErrorMessage", "You do not have the proper set up. In order to start adding morphs you need a valid Target Mesh"));
			
			MorphTargets.Empty();
			
		}
	}

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, bEditExistingMorph))
	{
		if (bWasEditingExitingMorph && !bEditExistingMorph)
		{
			// remove all morph because we are about to manually create new morph targets
			ParentToolPtr->RemoveAllMorphTargetMeshes();
			bWasEditingExitingMorph = false;
			Meshes = ParentToolPtr->GetMorphTargetMeshMap();
		}
		else if(!bWasEditingExitingMorph && bEditExistingMorph && MorphTargets.Num() > 0)
		{
			ParentToolPtr->RemoveAllMorphTargetMeshes();
			Meshes = ParentToolPtr->GetMorphTargetMeshMap();
			MorphTargets.Empty();
			bWasEditingExitingMorph = true;
			Meshes = ParentToolPtr->GetMorphTargetMeshMap();
		}
	}

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, MorphTargetToEdit))
	{
		if (MorphTargetToEdit.IsValid())
		{
			if (bOverrideExistingMorph)
			{
				ParentToolPtr->CloneMorph(MorphTargetToEdit, NAME_None);
				Meshes = ParentToolPtr->GetMorphTargetMeshMap();
			}
		}
	}

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, bOverrideExistingMorph))
	{
		if (MorphTargetToEdit.IsValid() && NewMorphTargetName.IsValid())
		{
			ParentToolPtr->RemoveAllMorphTargetMeshes();
			ParentToolPtr->CloneMorph(MorphTargetToEdit, NewMorphTargetName);
			Meshes = ParentToolPtr->GetMorphTargetMeshMap();
		}
	}

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, NewMorphTargetName))
	{
		if (!bOverrideExistingMorph && MorphTargetToEdit.IsValid() && NewMorphTargetName.IsValid())
		{
			ParentToolPtr->CloneMorph(MorphTargetToEdit, NewMorphTargetName);
			Meshes = ParentToolPtr->GetMorphTargetMeshMap();
		}
	}




	
}

void UMorphTargetProperties::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

#endif


UMorphTargetCreatorToolBuilder::UMorphTargetCreatorToolBuilder(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	AcceptedClasses.Add(UMaterialProvider::StaticClass());
	AcceptedClasses.Add(UDynamicMeshProvider::StaticClass());
	AcceptedClasses.Add(UDynamicMeshCommitter::StaticClass());
	MinRequiredMatches = 0;
	
}

UMorphTargetCreator::UMorphTargetCreator()
{
	ToolName = LOCTEXT("ToolName", "Morph Target Creator");
	ToolTooltip = LOCTEXT("ToolTooltip", "Generate morphs for any skeletal mesh");
	ToolCategory = LOCTEXT("ToolCategory", "Mesh Edit");
	ToolLongName = LOCTEXT("ToolLongName", "Morph Target Creator");
	ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
	ToolStartupRequirements = EScriptableToolStartupRequirements::Custom;
	CustomToolBuilderClass = UMorphTargetCreatorToolBuilder::StaticClass();
	bWantMouseHover = true;
}

void UMorphTargetCreator::SpawnActorInstance(const FTransform& SpawnTransform)
{
	GEditor->GetSelectedActors()->DeselectAll();

	auto World = GetToolWorld();
	
	if (GetHandyManAPI())
	{
		// Generate the splines from the input actor
		FActorSpawnParameters Params = FActorSpawnParameters();
		Params.ObjectFlags = RF_Transient;
		FString name = FString::Format(TEXT("Actor_{0}"), { "MorphTargetCreator" });
		FName fname = MakeUniqueObjectName(nullptr, AMorphTargetCreatorProxyActor::StaticClass(), FName(*name));
		Params.Name = fname;
		Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;

		auto ClassToSpawn = GetHandyManAPI()->GetPCGActorClass(FName(ToolName.ToString()));
		if (auto SpawnedActor =  World->SpawnActor<AMorphTargetCreatorProxyActor>(ClassToSpawn, Params))
		{
			// Initalize the actor
			SpawnedActor->SetActorTransform(SpawnTransform);
			TargetActor = (SpawnedActor);
		}
		
		// Build targets based on the target actor
		
		
		GEditor->SelectActor(TargetActor, true, true);
		
		GEditor->MoveViewportCamerasToActor(*TargetActor, true);

		GUnrealEd->edactHideUnselected(GetToolWorld() );
		// Then unhide selected to ensure that everything that's selected will be unhidden
		GUnrealEd->edactUnhideSelected(GetToolWorld());
		
		GEditor->GetSelectedActors()->DeselectAll();
		
		DynamicMeshComponent = NewObject<UDynamicMeshComponent>(TargetActor);

		ToolSetupUtil::ApplyRenderingConfigurationToPreview(DynamicMeshComponent, nullptr);

		// disable shadows initially, as changing shadow settings invalidates the SceneProxy
		DynamicMeshComponent->SetShadowsEnabled(false);
		DynamicMeshComponent->SetupAttachment(TargetActor->GetRootComponent());
		DynamicMeshComponent->RegisterComponent();
	

		InitializeSculptMeshComponent();
		
		
		/*for (const auto& Viewport : GUnrealEd->GetAllViewportClients())
		{
			if(!Viewport->IsFocused(GEditor->GetActiveViewport())) continue;
			Viewport->SetViewMode(VMI_Unlit);
		}*/
	}
	else
	{
		// TODO : Error Dialogue
	}
}

void UMorphTargetCreator::InitializeSculptMeshComponent()
{
	
	// initialize from LOD-0 MeshDescription
	
	double MaxDimension = DynamicMeshComponent->GetMesh()->GetBounds(true).MaxDim();
	
	// bake rotation and scaling into mesh because handling these inside sculpting is a mess
	// Note: this transform does not include translation ( so only the 3x3 transform)
	InitialTargetTransform = (FTransform3d)TargetActor->GetRootComponent()->GetComponentTransform();
	// clamp scaling because if we allow zero-scale we cannot invert this transform on Accept
	InitialTargetTransform.ClampMinimumScale(0.01);
	FVector3d Translation = InitialTargetTransform.GetTranslation();
	InitialTargetTransform.SetTranslation(FVector3d::Zero());
	DynamicMeshComponent->ApplyTransform(InitialTargetTransform, false);
	CurTargetTransform = FTransform3d(Translation);
	DynamicMeshComponent->SetWorldTransform((FTransform)CurTargetTransform);

	
	// First hide unselected as this will also hide group actor members
	//GUnrealEd->edactHideUnselected( GetToolWorld() );
	// Then unhide selected to ensure that everything that's selected will be unhidden
	//GUnrealEd->edactUnhideSelected(GetToolWorld());


	//GEditor->GetSelectedActors()->DeselectAll();
	
}


/*
 * internal Change classes
 */

class FMorphTargetNonSymmetricChange : public FToolCommandChange
{
public:
	virtual void Apply(UObject* Object) override;
	virtual void Revert(UObject* Object) override;
};



/*
 * Tool
 */

void UMorphTargetCreator::TriggerToolStartUp()
{
	if (DynamicMeshComponent != nullptr)
	{
		DynamicMeshComponent->OnMeshChanged.Remove(OnDynamicMeshComponentChangedHandle);
	}
	
	bIsReadyToSculpt = false;
	// create dynamic mesh component to use for live preview
	check(GetToolWorld());
	
	InitializeSculptMeshComponent();

	for (int k = 0; k < MorphTargetProperties->TargetMesh->GetMaterials().Num(); ++k)
	{
		UMaterialInterface* Mat = MorphTargetProperties->TargetMesh->GetMaterials()[k].MaterialInterface;
		DynamicMeshComponent->SetMaterial(k, Mat);
	}

	// assign materials
	FComponentMaterialSet MaterialSet;
	//Cast<IMaterialProvider>()->GetMaterialSet(MaterialSet);
	
	DynamicMeshComponent->SetInvalidateProxyOnChangeEnabled(false);
	OnDynamicMeshComponentChangedHandle = DynamicMeshComponent->OnMeshVerticesChanged.AddUObject(this, &UMorphTargetCreator::OnDynamicMeshComponentChanged);

	FDynamicMesh3* SculptMesh = GetSculptMesh();
	FAxisAlignedBox3d Bounds = SculptMesh->GetBounds(true);

	// initialize dynamic octree
	TFuture<void> InitializeOctree = Async(VertexSculptToolAsyncExecTarget, [SculptMesh, Bounds, this]()
	{
		if (SculptMesh->TriangleCount() > 100000)
		{
			Octree.RootDimension = Bounds.MaxDim() / 10.0;
			Octree.SetMaxTreeDepth(4);
		}
		else
		{
			Octree.RootDimension = Bounds.MaxDim() / 2.0;
			Octree.SetMaxTreeDepth(8);
		}
		Octree.Initialize(SculptMesh);
		//Octree.CheckValidity(EValidityCheckFailMode::Check, true, true);
		//FDynamicMeshOctree3::FStatistics Stats;
		//Octree.ComputeStatistics(Stats);
		//UE_LOG(LogTemp, Warning, TEXT("Octree Stats: %s"), *Stats.ToString());
	});

	// find mesh connected-component index for each triangle
	TFuture<void> InitializeComponents = Async(VertexSculptToolAsyncExecTarget, [SculptMesh, this]()
	{
		TriangleComponentIDs.SetNum(SculptMesh->MaxTriangleID());
		FMeshConnectedComponents Components(SculptMesh);
		Components.FindConnectedTriangles();
		int32 ComponentIdx = 1;
		for (const FMeshConnectedComponents::FComponent& Component : Components)
		{
			for (int32 TriIdx : Component.Indices)
			{
				TriangleComponentIDs[TriIdx] = ComponentIdx;
			}
			ComponentIdx++;
		}
	});

	TFuture<void> InitializeSymmetry = Async(VertexSculptToolAsyncExecTarget, [SculptMesh, this]()
	{
		TryToInitializeSymmetry();
	});

	// currently only supporting default polygroup set
	TFuture<void> InitializeGroups = Async(VertexSculptToolAsyncExecTarget, [SculptMesh, this]()
	{
		ActiveGroupSet = MakeUnique<UE::Geometry::FPolygroupSet>(SculptMesh);
	});

	// initialize target mesh
	TFuture<void> InitializeBaseMesh = Async(VertexSculptToolAsyncExecTarget, [this]()
	{
		UpdateBaseMesh(nullptr);
		bTargetDirty = false;
	});

	// initialize render decomposition
	TFuture<void> InitializeRenderDecomp = Async(VertexSculptToolAsyncExecTarget, [SculptMesh, &MaterialSet, this]()
	{
		TUniquePtr<FMeshRenderDecomposition> Decomp = MakeUnique<FMeshRenderDecomposition>();
		FMeshRenderDecomposition::BuildChunkedDecomposition(SculptMesh, &MaterialSet, *Decomp);
		Decomp->BuildAssociations(SculptMesh);
		//UE_LOG(LogTemp, Warning, TEXT("Decomposition has %d groups"), Decomp->Num());
		DynamicMeshComponent->SetExternalDecomposition(MoveTemp(Decomp));
	});

	// Wait for above precomputations to finish before continuing
	InitializeOctree.Wait();
	InitializeComponents.Wait();
	InitializeGroups.Wait();
	InitializeBaseMesh.Wait();
	InitializeRenderDecomp.Wait();
	InitializeSymmetry.Wait();

	// initialize brush radius range interval, brush properties
	InitializeBrushSizeRange(Bounds);
	
	
	GizmoProperties->RecenterGizmoIfFar(GetSculptMeshComponent()->GetComponentTransform().TransformPosition(Bounds.Center()), Bounds.MaxDim());

	
	bIsReadyToSculpt = true;
}

void UMorphTargetCreator::UpdateMaterialMode(EHandyManMeshEditingMaterialModes NewMode)
{
	if (NewMode == EHandyManMeshEditingMaterialModes::ExistingMaterial)
	{
		GetSculptMeshComponent()->ClearOverrideRenderMaterial();
		GetSculptMeshComponent()->SetShadowsEnabled(DynamicMeshComponent->bCastDynamicShadow);
		ActiveOverrideMaterial = nullptr;
	}
	else
	{
		if (NewMode == EHandyManMeshEditingMaterialModes::Custom)
		{
			if (ViewProperties->CustomMaterial.IsValid())
			{
				ActiveOverrideMaterial = UMaterialInstanceDynamic::Create(ViewProperties->CustomMaterial.Get(), this);
			}
			else
			{
				GetSculptMeshComponent()->ClearOverrideRenderMaterial();
				ActiveOverrideMaterial = nullptr;
			}
		}
		else if (NewMode == EHandyManMeshEditingMaterialModes::CustomImage)
		{
			ActiveOverrideMaterial = ToolSetupUtil::GetCustomImageBasedSculptMaterial(GetToolManager(), ViewProperties->Image);
			if (ViewProperties->Image != nullptr)
			{
				ActiveOverrideMaterial->SetTextureParameterValue(TEXT("ImageTexture"), ViewProperties->Image);
			}
		}
		else if (NewMode == EHandyManMeshEditingMaterialModes::VertexColor)
		{
			ActiveOverrideMaterial = ToolSetupUtil::GetVertexColorMaterial(GetToolManager());
		}
		else if (NewMode == EHandyManMeshEditingMaterialModes::Transparent)
		{
			ActiveOverrideMaterial = ToolSetupUtil::GetTransparentSculptMaterial(GetToolManager(), 
				ViewProperties->TransparentMaterialColor, ViewProperties->Opacity, ViewProperties->bTwoSided);
		}
		else
		{
			UMaterialInterface* SculptMaterial = nullptr;
			switch (NewMode)
			{
			case EHandyManMeshEditingMaterialModes::Diffuse:
				SculptMaterial = ToolSetupUtil::GetDefaultSculptMaterial(GetToolManager());
				break;
			case EHandyManMeshEditingMaterialModes::Grey:
				SculptMaterial = ToolSetupUtil::GetImageBasedSculptMaterial(GetToolManager(), ToolSetupUtil::ImageMaterialType::DefaultBasic);
				break;
			case EHandyManMeshEditingMaterialModes::Soft:
				SculptMaterial = ToolSetupUtil::GetImageBasedSculptMaterial(GetToolManager(), ToolSetupUtil::ImageMaterialType::DefaultSoft);
				break;
			case EHandyManMeshEditingMaterialModes::TangentNormal:
				SculptMaterial = ToolSetupUtil::GetImageBasedSculptMaterial(GetToolManager(), ToolSetupUtil::ImageMaterialType::TangentNormalFromView);
				break;
			}
			if (SculptMaterial != nullptr )
			{
				ActiveOverrideMaterial = UMaterialInstanceDynamic::Create(SculptMaterial, this);
			}
		}

		if (ActiveOverrideMaterial != nullptr)
		{
			GetSculptMeshComponent()->SetOverrideRenderMaterial(ActiveOverrideMaterial);
			ActiveOverrideMaterial->SetScalarParameterValue(TEXT("FlatShading"), (ViewProperties->bFlatShading) ? 1.0f : 0.0f);
		}

		GetSculptMeshComponent()->SetShadowsEnabled(false);
	}
}

void UMorphTargetCreator::InitializeToolPallette(EToolsFrameworkOutcomePins PropertyCreationOutcome)
{
	// initialize other properties
	SculptProperties = Cast<UMorphTargetBrushSculptProperties>(AddPropertySetOfType(UMorphTargetBrushSculptProperties::StaticClass(), "SculptProperties", PropertyCreationOutcome));
	
	
	// init state flags flags
	ActiveVertexChange = nullptr;

	InitializeIndicator();

	// initialize our properties
	BrushProperties = Cast<UHandyManSculptBrushProperties>(AddPropertySetOfType(UHandyManSculptBrushProperties::StaticClass(), "BrushProperties", PropertyCreationOutcome));
	BrushProperties->bShowPerBrushProps = false;
	BrushProperties->bShowFalloff = false;
	if (SharesBrushPropertiesChanges())
	{
		BrushProperties->RestoreProperties(this);
	}
	CalculateBrushRadius();

	AlphaProperties = Cast<UMorphTargetBrushAlphaProperties>(AddPropertySetOfType(UMorphTargetBrushAlphaProperties::StaticClass(), "AlphaProperties", PropertyCreationOutcome));
	AlphaProperties->RestoreProperties(this);

	SymmetryProperties = Cast<UMorphTargetMeshSymmetryProperties>(AddPropertySetOfType(UMorphTargetMeshSymmetryProperties::StaticClass(), "SymmetryProperties", PropertyCreationOutcome));
	SymmetryProperties->RestoreProperties(this);
	SymmetryProperties->bSymmetryCanBeEnabled = false;

	
	RegisterBrushType((int32)EHandyManBrushType::Smooth, LOCTEXT("SmoothBrush", "Smooth"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FHandyManSmoothBrushOp>>(),
	                  NewObject<UHandyManSmoothBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::SmoothFill, LOCTEXT("SmoothFill", "SmoothFill"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FHandyManSmoothFillBrushOp>>(),
	                  NewObject<UHandyManSmoothFillBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::Move, LOCTEXT("Move", "Move"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FHandyManMoveBrushOp>>(),
	                  NewObject<UHandyManMoveBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::Offset, LOCTEXT("Offset", "SculptN"),
	                  MakeUnique<FHandyManLambdaMeshSculptBrushOpFactory>([this]() { return MakeUnique<FHandyManSurfaceSculptBrushOp>(BaseMeshQueryFunc); }),
	                  NewObject<UHandyManStandardSculptBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::SculptView, LOCTEXT("SculptView", "SculptV"),
	                  MakeUnique<FHandyManLambdaMeshSculptBrushOpFactory>( [this]() { return MakeUnique<FHandyManViewAlignedSculptBrushOp>(BaseMeshQueryFunc); } ),
	                  NewObject<UHandyManViewAlignedSculptBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::SculptMax, LOCTEXT("SculptMax", "SculptMx"),
	                  MakeUnique<FHandyManLambdaMeshSculptBrushOpFactory>([this]() { return MakeUnique<FHandyManSurfaceMaxSculptBrushOp>(BaseMeshQueryFunc); }),
	                  NewObject<UHandyManSculptMaxBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::Inflate, LOCTEXT("Inflate", "Inflate"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FHandyManInflateBrushOp>>(),
	                  NewObject<UHandyManInflateBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::Pinch, LOCTEXT("Pinch", "Pinch"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FHandyManPinchBrushOp>>(),
	                  NewObject<UHandyManPinchBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::Flatten, LOCTEXT("Flatten", "Flatten"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FHandyManFlattenBrushOp>>(),
	                  NewObject<UHandyManFlattenBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::Plane, LOCTEXT("Plane", "PlaneN"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FHandyManPlaneBrushOp>>(),
	                  NewObject<UHandyManPlaneBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::PlaneViewAligned, LOCTEXT("PlaneViewAligned", "PlaneV"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FHandyManPlaneBrushOp>>(),
	                  NewObject<UHandyManViewAlignedPlaneBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::FixedPlane, LOCTEXT("FixedPlane", "PlaneW"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FHandyManPlaneBrushOp>>(),
	                  NewObject<UHandyManFixedPlaneBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::ScaleKelvin , LOCTEXT("ScaleKelvin", "Scale"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FScaleHandyManKelvinletBrushOp>>(),
	                  NewObject<UScaleHandyManKelvinletBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::PullKelvin, LOCTEXT("PullKelvin", "Grab"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FPullHandyManKelvinletBrushOp>>(),
	                  NewObject<UPullHandyManKelvinletBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::PullSharpKelvin, LOCTEXT("PullSharpKelvin", "GrabSharp"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FSharpPullHandyManKelvinletBrushOp>>(),
	                  NewObject<USharpPullHandyManKelvinletBrushOpProps>(this));

	RegisterBrushType((int32)EHandyManBrushType::TwistKelvin, LOCTEXT("TwistKelvin", "Twist"),
	                  MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FTwistHandyManKelvinletBrushOp>>(),
	                  NewObject<UTwistHandyManKelvinletBrushOpProps>(this));

	// secondary brushes
	RegisterSecondaryBrushType((int32)EHandyManBrushType::Smooth, LOCTEXT("Smooth", "Smooth"),
	                           MakeUnique<THandyManBasicMeshSculptBrushOpFactory<FHandyManSmoothBrushOp>>(),
	                           NewObject<UHandyManSecondarySmoothBrushOpProps>(this));

	// falloffs
	RegisterStandardFalloffTypes();


	GizmoProperties = Cast<UHandyManWorkPlaneProperties>(AddPropertySetOfType(UHandyManWorkPlaneProperties::StaticClass(), "GizmoProperties", PropertyCreationOutcome));
	SetToolPropertySourceEnabled(GizmoProperties, false);
	// Move the gizmo toward the center of the mesh, without changing the plane it represents


	// register watchers
	SculptProperties->WatchProperty( SculptProperties->PrimaryBrushType,
	                                 [this](EHandyManBrushType NewType) { UpdateBrushType(NewType); });

	SculptProperties->WatchProperty( SculptProperties->PrimaryFalloffType,
	                                 [this](EHandyManMeshSculptFalloffType NewType) { 
		                                 SetPrimaryFalloffType(NewType);
		                                 // Request to have the details panel rebuilt to ensure the new falloff property value is propagated to the details customization
		                                 OnDetailsPanelRequestRebuild.Broadcast();
	                                 });

	SculptProperties->WatchProperty(AlphaProperties->Alpha,
	                                [this](UTexture2D* NewAlpha) {
		                                UpdateBrushAlpha(AlphaProperties->Alpha);
		                                // Request to have the details panel rebuilt to ensure the new alpha property value is propagated to the details customization
		                                OnDetailsPanelRequestRebuild.Broadcast();
	                                });

	// must call before updating brush type so that we register all brush properties?
	OnCompleteSetup();

	UpdateBrushType(SculptProperties->PrimaryBrushType);
	SetPrimaryFalloffType(SculptProperties->PrimaryFalloffType);
	UpdateBrushAlpha(AlphaProperties->Alpha);
	SetActiveSecondaryBrushType((int32)EHandyManBrushType::Smooth);

	StampRandomStream = FRandomStream(31337);

	// update symmetry state based on validity, and then update internal apply-symmetry state
	SymmetryProperties->bSymmetryCanBeEnabled = bMeshSymmetryIsValid;
	bApplySymmetry = bMeshSymmetryIsValid && SymmetryProperties->bEnableSymmetry;

	SymmetryProperties->WatchProperty(SymmetryProperties->bEnableSymmetry,
	                                  [this](bool bNewValue) { bApplySymmetry = bMeshSymmetryIsValid && bNewValue; });
}

void UMorphTargetCreator::Setup()
{

	EToolsFrameworkOutcomePins PropertyCreationOutcome;
	MorphTargetProperties = Cast<UMorphTargetProperties>(AddPropertySetOfType(UMorphTargetProperties::StaticClass(), "MorphTargetProperties", PropertyCreationOutcome));

	Super::Setup();
	
	MorphTargetProperties->WatchProperty(MorphTargetProperties->TargetMesh, [this](TObjectPtr<USkeletalMesh>)
	{
		if (IsValid(MorphTargetProperties->TargetMesh) && TargetActor)
		{
			TargetActor->CacheBaseMesh(MorphTargetProperties->TargetMesh);
			TriggerToolStartUp();
			bHasToolStarted = true;
		}
	});

	SpawnActorInstance(FTransform::Identity);
	
	InitializeToolPallette(PropertyCreationOutcome);

	
	this->BaseMeshQueryFunc = [&](int32 VertexID, const FVector3d& Position, double MaxDist, FVector3d& PosOut, FVector3d& NormalOut)
	{
		return GetBaseMeshNearest(VertexID, Position, MaxDist, PosOut, NormalOut);
	};
	
}

void UMorphTargetCreator::Shutdown(EToolShutdownType ShutdownType)
{
	if (DynamicMeshComponent != nullptr)
	{
		DynamicMeshComponent->OnMeshChanged.Remove(OnDynamicMeshComponentChangedHandle);
	}

	GEditor->GetSelectedActors()->DeselectAll();

	GUnrealEd->edactUnHideAll(GetToolWorld());

	SculptProperties->SaveProperties(this);
	AlphaProperties->SaveProperties(this);
	SymmetryProperties->SaveProperties(this);

	switch (ShutdownType)
	{
	case EToolShutdownType::Completed:
		break;
	case EToolShutdownType::Accept:
		if (TargetActor != nullptr)
		{
			TargetActor->SaveObject(DynamicMeshComponent->GetDynamicMesh());
			TargetActor = nullptr;
		}
		break;
	case EToolShutdownType::Cancel:
		if (TargetActor != nullptr)
		{
			TargetActor->Destroy();
			TargetActor = nullptr;
		}
		break;
	}

	// this call will commit result, unregister and destroy DynamicMeshComponent
	Super::Shutdown(ShutdownType);
}


void UMorphTargetCreator::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{
	if(!bHasToolStarted && !TargetActor) return;

	CalculateBrushRadius();
}

bool UMorphTargetCreator::SupportsWorldSpaceFocusBox()
{
	return TargetActor && Cast<UPrimitiveComponentToolTarget>(TargetActor) != nullptr;
}

FBox UMorphTargetCreator::GetWorldSpaceFocusBox()
{
	return Super::GetWorldSpaceFocusBox();
}

TMap<FName, UDynamicMesh*> UMorphTargetCreator::GetMorphTargetMeshMap() const
{
	if (TargetActor)
	{
		return TargetActor->GetMorphTargetMeshMap();
	}

	return TMap<FName, UDynamicMesh*>();
}

void UMorphTargetCreator::RemoveMorphTargetMesh(const FName MorphTargetMeshName)
{
	if (TargetActor)
	{
		const bool ShouldRestoreMesh = CurrentMorphEdit.IsEqual(MorphTargetMeshName);
		TargetActor->RemoveMorphTargetMesh(MorphTargetMeshName);
		
		if (ShouldRestoreMesh)
		{
			TargetActor->RestoreLastMorphTarget();
		}
	}
}

void UMorphTargetCreator::RemoveAllMorphTargetMeshes(const bool bShouldRestoreLastMorph)
{
	if (TargetActor)
	{
		TargetActor->RemoveAllMorphTargetMeshes(bShouldRestoreLastMorph);
	}
}

void UMorphTargetCreator::CreateMorphTargetMesh(FName MorphTargetMeshName)
{
	if (TargetActor)
	{
		TargetActor->CreateMorphTargetMesh(MorphTargetMeshName);
		CurrentMorphEdit = MorphTargetMeshName;
	}
}

void UMorphTargetCreator::CloneMorph(const FName MorphTargetName, const FName NewMorphTargetName)
{
	if (TargetActor)
	{
		TargetActor->CloneMorphTarget(MorphTargetName, NewMorphTargetName);
		CurrentMorphEdit = NewMorphTargetName.IsValid() ? NewMorphTargetName : MorphTargetName;
	}
}


UPreviewMesh* UMorphTargetCreator::MakeBrushIndicatorMesh(UObject* Parent, UWorld* World)
{
	if(!bHasToolStarted && !TargetActor) return nullptr;

	UPreviewMesh* PlaneMesh = NewObject<UPreviewMesh>(Parent);
	PlaneMesh->CreateInWorld(World, FTransform::Identity);

	FRectangleMeshGenerator RectGen;
	RectGen.Width = RectGen.Height = 2.0;
	RectGen.WidthVertexCount = RectGen.HeightVertexCount = 1;
	FDynamicMesh3 Mesh(&RectGen.Generate());
	FDynamicMeshUVOverlay* UVOverlay = Mesh.Attributes()->PrimaryUV();
	// configure UVs to be in same space as texture pixels when mapped into brush frame (??)
	for (int32 eid : UVOverlay->ElementIndicesItr())
	{
		FVector2f UV = UVOverlay->GetElement(eid);
		UV.X = 1.0 - UV.X;
		UV.Y = 1.0 - UV.Y;
		UVOverlay->SetElement(eid, UV);
	}
	PlaneMesh->UpdatePreview(&Mesh);

	BrushIndicatorMaterial = ToolSetupUtil::GetDefaultBrushAlphaMaterial(GetToolManager());
	if (BrushIndicatorMaterial)
	{
		PlaneMesh->SetMaterial(BrushIndicatorMaterial);
	}

	// make sure raytracing is disabled on the brush indicator
	Cast<UDynamicMeshComponent>(PlaneMesh->GetRootComponent())->SetEnableRaytracing(false);
	PlaneMesh->SetShadowsEnabled(false);

	return PlaneMesh;
}

void UMorphTargetCreator::InitializeIndicator()
{
	if(!bHasToolStarted && !TargetActor) return;

	Super::InitializeIndicator();
	// want to draw radius
	BrushIndicator->bDrawRadiusCircle = true;
}

void UMorphTargetCreator::SetActiveBrushType(int32 Identifier)
{
	if(!bHasToolStarted && !TargetActor) return;

	EHandyManBrushType NewBrushType =  static_cast<EHandyManBrushType>(Identifier);
	if (SculptProperties->PrimaryBrushType != NewBrushType)
	{
		SculptProperties->PrimaryBrushType = NewBrushType;
		UpdateBrushType(SculptProperties->PrimaryBrushType);
		SculptProperties->SilentUpdateWatched();
	}

	// this forces full rebuild of properties panel (!!)
	//this->NotifyOfPropertyChangeByTool(SculptProperties);
}

void UMorphTargetCreator::SetActiveFalloffType(int32 Identifier)
{
	if(!bHasToolStarted && !TargetActor) return;

	EHandyManMeshSculptFalloffType NewFalloffType = static_cast<EHandyManMeshSculptFalloffType>(Identifier);
	if (SculptProperties->PrimaryFalloffType != NewFalloffType)
	{
		SculptProperties->PrimaryFalloffType = NewFalloffType;
		SetPrimaryFalloffType(SculptProperties->PrimaryFalloffType);
		SculptProperties->SilentUpdateWatched();
	}

	// this forces full rebuild of properties panel (!!)
	//this->NotifyOfPropertyChangeByTool(SculptProperties);
}

void UMorphTargetCreator::SetRegionFilterType(int32 Identifier)
{
	if(!bHasToolStarted && !TargetActor) return;

	SculptProperties->BrushFilter = static_cast<EHandyManBrushFilterType>(Identifier);
}


void UMorphTargetCreator::OnBeginStroke(const FRay& WorldRay)
{
	if(!bHasToolStarted && !TargetActor) return;

	WaitForPendingUndoRedo();		// cannot start stroke if there is an outstanding undo/redo update

	UpdateBrushPosition(WorldRay);

	if (SculptProperties->PrimaryBrushType == EHandyManBrushType::Plane ||
		SculptProperties->PrimaryBrushType == EHandyManBrushType::PlaneViewAligned)
	{
		UpdateROI(GetBrushFrameLocal().Origin);
		UpdateStrokeReferencePlaneForROI(GetBrushFrameLocal(), TriangleROIArray,
			(SculptProperties->PrimaryBrushType == EHandyManBrushType::PlaneViewAligned));
	}
	else if (SculptProperties->PrimaryBrushType == EHandyManBrushType::FixedPlane)
	{
		UpdateStrokeReferencePlaneFromWorkPlane();
	}

	// initialize first "Last Stamp", so that we can assume all stamps in stroke have a valid previous stamp
	LastStamp.WorldFrame = GetBrushFrameWorld();
	LastStamp.LocalFrame = GetBrushFrameLocal();
	LastStamp.Radius = GetCurrentBrushRadius();
	LastStamp.Falloff = GetCurrentBrushFalloff();
	LastStamp.Direction = GetInInvertStroke() ? -1.0 : 1.0;
	LastStamp.Depth = GetCurrentBrushDepth();
	LastStamp.Power = GetActivePressure() * GetCurrentBrushStrength();
	LastStamp.TimeStamp = FDateTime::Now();

	// If applying symmetry, make sure the stamp is on the "positive" side. 
	if (bApplySymmetry)
	{
		LastStamp.LocalFrame = Symmetry->GetPositiveSideFrame(LastStamp.LocalFrame);
		LastStamp.WorldFrame = LastStamp.LocalFrame;
		LastStamp.WorldFrame.Transform(CurTargetTransform);
	}

	InitialStrokeTriangleID = -1;
	InitialStrokeTriangleID = GetBrushTriangleID();

	FHandyManSculptBrushOptions SculptOptions;
	//SculptOptions.bPreserveUVFlow = false; // SculptProperties->bPreserveUVFlow;
	SculptOptions.ConstantReferencePlane = GetCurrentStrokeReferencePlane();

	TUniquePtr<FHandyManMeshSculptBrushOp>& UseBrushOp = GetActiveBrushOp();
	UseBrushOp->ConfigureOptions(SculptOptions);
	UseBrushOp->BeginStroke(GetSculptMesh(), LastStamp, VertexROI);

	AccumulatedTriangleROI.Reset();

	// begin change here? or wait for first stamp?
	BeginChange();
}

void UMorphTargetCreator::OnEndStroke()
{
	if(!bHasToolStarted && !TargetActor) return;
	// update spatial
	bTargetDirty = true;

	GetActiveBrushOp()->EndStroke(GetSculptMesh(), LastStamp, VertexROI);

	// close change record
	EndChange();
}


void UMorphTargetCreator::OnCancelStroke()
{
	if(!bHasToolStarted && !TargetActor) return;
	
	GetActiveBrushOp()->CancelStroke();

	delete ActiveVertexChange;
	ActiveVertexChange = nullptr;
}



void UMorphTargetCreator::UpdateROI(const FVector3d& BrushPos)
{
	if(!bHasToolStarted && !TargetActor) return;
	TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_UpdateROI);

	float RadiusSqr = GetCurrentBrushRadius() * GetCurrentBrushRadius();
	FAxisAlignedBox3d BrushBox(
		BrushPos - GetCurrentBrushRadius() * FVector3d::One(),
		BrushPos + GetCurrentBrushRadius() * FVector3d::One());

	// do a parallel range quer
	RangeQueryTriBuffer.Reset();
	FDynamicMesh3* Mesh = GetSculptMesh();
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_UpdateROI_RangeQuery);
		Octree.ParallelRangeQuery(BrushBox, RangeQueryTriBuffer);
	}

	int32 ActiveComponentID = -1;
	int32 ActiveGroupID = -1;
	if (SculptProperties->BrushFilter == EHandyManBrushFilterType::Component)
	{
		ActiveComponentID = (InitialStrokeTriangleID >= 0 && InitialStrokeTriangleID <= TriangleComponentIDs.Num()) ?
			TriangleComponentIDs[InitialStrokeTriangleID] : -1;
	}
	else if (SculptProperties->BrushFilter == EHandyManBrushFilterType::PolyGroup)
	{
		ActiveGroupID = Mesh->IsTriangle(InitialStrokeTriangleID) ? ActiveGroupSet->GetGroup(InitialStrokeTriangleID) : -1;
	}

#if 1
	// in this path we use more memory but this lets us do more in parallel

	// Construct array of inside/outside flags for each triangle's vertices. If no
	// vertices are inside, clear the triangle ID from the range query buffer.
	// This can be done in parallel and it's cheaper to do repeated distance computations
	// than to try to do it inside the ROI building below (todo: profile this some more?)
	TriangleROIInBuf.SetNum(RangeQueryTriBuffer.Num(), EAllowShrinking::No);
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(DynamicMeshSculptTool_UpdateROI_TriVerts);
		ParallelFor(RangeQueryTriBuffer.Num(), [&](int k)
		{
			// check various triangle ROI filters
			int32 tid = RangeQueryTriBuffer[k];
			bool bDiscardTriangle = false;
			if (ActiveComponentID >= 0 && TriangleComponentIDs[tid] != ActiveComponentID)
			{
				bDiscardTriangle = true;
			}
			if (ActiveGroupID >= 0 && ActiveGroupSet->GetGroup(tid) != ActiveGroupID)
			{
				bDiscardTriangle = true;
			}
			if (bDiscardTriangle)
			{
				TriangleROIInBuf[k].A = TriangleROIInBuf[k].B = TriangleROIInBuf[k].C = 0;
				RangeQueryTriBuffer[k] = -1;
				return;
			}

			const FIndex3i& TriV = Mesh->GetTriangleRef(tid);
			TriangleROIInBuf[k].A = (DistanceSquared(BrushPos, Mesh->GetVertexRef(TriV.A)) < RadiusSqr) ? 1 : 0;
			TriangleROIInBuf[k].B = (DistanceSquared(BrushPos, Mesh->GetVertexRef(TriV.B)) < RadiusSqr) ? 1 : 0;
			TriangleROIInBuf[k].C = (DistanceSquared(BrushPos, Mesh->GetVertexRef(TriV.C)) < RadiusSqr) ? 1 : 0;
			if (TriangleROIInBuf[k].A + TriangleROIInBuf[k].B + TriangleROIInBuf[k].C == 0)
			{
				RangeQueryTriBuffer[k] = -1;
			}
		});
	}

	// Build up vertex and triangle ROIs from the remaining range-query triangles.
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(DynamicMeshSculptTool_UpdateROI_3Collect);
		VertexROIBuilder.Initialize(Mesh->MaxVertexID());
		TriangleROIBuilder.Initialize(Mesh->MaxTriangleID());
		int32 N = RangeQueryTriBuffer.Num();
		for ( int32 k = 0; k < N; ++k )
		{
			int32 tid = RangeQueryTriBuffer[k];
			if (tid == -1) continue;		// triangle was deleted in previous step
			const FIndex3i& TriV = Mesh->GetTriangleRef(RangeQueryTriBuffer[k]);
			const FIndex3i& Inside = TriangleROIInBuf[k];
			int InsideCount = 0;
			for (int j = 0; j < 3; ++j)
			{
				if (Inside[j])
				{
					VertexROIBuilder.Add(TriV[j]);
					InsideCount++;
				}
			}
			if (InsideCount > 0)
			{
				TriangleROIBuilder.Add(tid);
			}
		}

		VertexROIBuilder.SwapValuesWith(VertexROI);

		if (bApplySymmetry)
		{
			// Find symmetric Vertex ROI. This will overlap with VertexROI in many cases.
			SymmetricVertexROI.Reset();
			Symmetry->GetMirrorVertexROI(VertexROI, SymmetricVertexROI, true);
			// expand the Triangle ROI to include the symmetric vertex one-rings
			for (int32 VertexID : SymmetricVertexROI)
			{
				if (Mesh->IsVertex(VertexID))
				{
					Mesh->EnumerateVertexTriangles(VertexID, [&](int32 tid)
					{
						TriangleROIBuilder.Add(tid);
					});
				}
			}
		}

		TriangleROIBuilder.SwapValuesWith(TriangleROIArray);
	}

#else
	// In this path we combine everything into one loop. Does fewer distance checks
	// but nothing can be done in parallel (would change if ROIBuilders had atomic-try-add)

	check(bApplySymmetry == false);		// have not implemented this path yet...

	// TODO would need to support these, this branch is likely dead though
	ensure(SculptProperties->BrushFilter == EMorphTargetBrushFilterType::None);

	// collect set of vertices and triangles inside brush sphere, from range query result
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(DynamicMeshSculptTool_UpdateROI_2Collect);
		VertexROIBuilder.Initialize(Mesh->MaxVertexID());
		TriangleROIBuilder.Initialize(Mesh->MaxTriangleID());
		for (int32 TriIdx : RangeQueryTriBuffer)
		{
			FIndex3i TriV = Mesh->GetTriangle(TriIdx);
			int InsideCount = 0;
			for (int j = 0; j < 3; ++j)
			{
				if (VertexROIBuilder.Contains(TriV[j]))
				{
					InsideCount++;
				} 
				else if (BrushPos.DistanceSquared(Mesh->GetVertexRef(TriV[j])) < RadiusSqr)
				{
					VertexROIBuilder.Add(TriV[j]);
					InsideCount++;
				}
			}
			if (InsideCount > 0)
			{
				TriangleROIBuilder.Add(tid);
			}
		}
		VertexROIBuilder.SwapValuesWith(VertexROI);
		TriangleROIBuilder.SwapValuesWith(TriangleROIArray);
	}
#endif

	{
		// set up and populate position buffers for Vertex ROI
		TRACE_CPUPROFILER_EVENT_SCOPE(DynamicMeshSculptTool_UpdateROI_4ROI);
		int32 ROISize = VertexROI.Num();
		ROIPositionBuffer.SetNum(ROISize, EAllowShrinking::No);
		ROIPrevPositionBuffer.SetNum(ROISize, EAllowShrinking::No);
		ParallelFor(ROISize, [&](int i)
		{
			ROIPrevPositionBuffer[i] = Mesh->GetVertexRef(VertexROI[i]);
		});
		// do the same for the Symmetric Vertex ROI
		if (bApplySymmetry)
		{
			SymmetricROIPositionBuffer.SetNum(ROISize, EAllowShrinking::No);
			SymmetricROIPrevPositionBuffer.SetNum(ROISize, EAllowShrinking::No);
			ParallelFor(ROISize, [&](int i)
			{
				if ( Mesh->IsVertex(SymmetricVertexROI[i]) )
				{
					SymmetricROIPrevPositionBuffer[i] = Mesh->GetVertexRef(SymmetricVertexROI[i]);
				}
			});
		}
	}
}

bool UMorphTargetCreator::UpdateStampPosition(const FRay& WorldRay)
{
	if(!bHasToolStarted && !TargetActor) return false;
	TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_UpdateStampPosition);

	CalculateBrushRadius();

	TUniquePtr<FHandyManMeshSculptBrushOp>& UseBrushOp = GetActiveBrushOp();

	EHandyManSculptBrushOpTargetType TargetType = UseBrushOp->GetBrushTargetType();
	switch (TargetType)
	{
	case EHandyManSculptBrushOpTargetType::SculptMesh:
		UpdateBrushPositionOnSculptMesh(WorldRay, true);
		break;
	case EHandyManSculptBrushOpTargetType::TargetMesh:
		UpdateBrushPositionOnTargetMesh(WorldRay, true);
		break;
	case EHandyManSculptBrushOpTargetType::ActivePlane:
		UpdateBrushPositionOnActivePlane(WorldRay);
		break;
	}

	if (UseBrushOp->GetAlignStampToView())
	{
		AlignBrushToView();
	}

	CurrentStamp = LastStamp;
	//CurrentStamp.DeltaTime = FMathd::Min((FDateTime::Now() - LastStamp.TimeStamp).GetTotalSeconds(), 1.0);
	CurrentStamp.DeltaTime = 0.03;		// 30 fps - using actual time is no good now that we support variable stamps!
	CurrentStamp.WorldFrame = GetBrushFrameWorld();
	CurrentStamp.LocalFrame = GetBrushFrameLocal();
	CurrentStamp.Power = GetActivePressure() * GetCurrentBrushStrength();

	if (bHaveBrushAlpha && (AlphaProperties->RotationAngle != 0 || AlphaProperties->bRandomize))
	{
		float UseAngle = AlphaProperties->RotationAngle;
		if (AlphaProperties->bRandomize)
		{
			UseAngle += (StampRandomStream.GetFraction() - 0.5f) * 2.0f * AlphaProperties->RandomRange;
		}

		// possibly should be done in base brush...
		CurrentStamp.WorldFrame.Rotate(FQuaterniond(CurrentStamp.WorldFrame.Z(), UseAngle, true));
		CurrentStamp.LocalFrame.Rotate(FQuaterniond(CurrentStamp.LocalFrame.Z(), UseAngle, true));
	}

	if (bApplySymmetry)
	{
		CurrentStamp.LocalFrame = Symmetry->GetPositiveSideFrame(CurrentStamp.LocalFrame);
		CurrentStamp.WorldFrame = CurrentStamp.LocalFrame;
		CurrentStamp.WorldFrame.Transform(CurTargetTransform);
	}

	CurrentStamp.PrevLocalFrame = LastStamp.LocalFrame;
	CurrentStamp.PrevWorldFrame = LastStamp.WorldFrame;

	FVector3d MoveDelta = CurrentStamp.LocalFrame.Origin - CurrentStamp.PrevLocalFrame.Origin;
	if (UseBrushOp->IgnoreZeroMovements() && MoveDelta.SquaredLength() < FMathd::ZeroTolerance)
	{
		return false;
	}

	return true;
}


TFuture<void> UMorphTargetCreator::ApplyStamp()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_ApplyStamp);

	TUniquePtr<FHandyManMeshSculptBrushOp>& UseBrushOp = GetActiveBrushOp();

	// compute region plane if necessary. This may currently be expensive?
	if (UseBrushOp->WantsStampRegionPlane())
	{
		CurrentStamp.RegionPlane = ComputeStampRegionPlane(CurrentStamp.LocalFrame, TriangleROIArray, true, false, false);
	}

	// set up alpha function if we have one
	if (bHaveBrushAlpha)
	{
		CurrentStamp.StampAlphaFunc = [this](const FHandyManSculptBrushStamp& Stamp, const FVector3d& Position)
		{
			return this->SampleBrushAlpha(Stamp, Position);
		};
	}

	// apply the stamp, which computes new positions
	FDynamicMesh3* Mesh = GetSculptMesh();
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_ApplyStamp_Apply);
		UseBrushOp->ApplyStamp(Mesh, CurrentStamp, VertexROI, ROIPositionBuffer);
	}

	// can discard alpha now
	CurrentStamp.StampAlphaFunc = nullptr;

	// if we are applying symmetry, we need to update the on-plane positions as they
	// will not be in the SymmetricVertexROI
	if (bApplySymmetry)
	{
		// update position of vertices that are on the symmetry plane
		Symmetry->ApplySymmetryPlaneConstraints(VertexROI, ROIPositionBuffer);

		// currently something gross is that VertexROI/ROIPositionBuffer may have both a vertex and it's mirror vertex,
		// each with a different position. We somehow need to be able to resolve this, but we don't have the mapping 
		// between the two locations in VertexROI, and we have no way to figure out the 'new' position of that mirror vertex
		// until we can look it up by VertexID, not array-index. So, we are going to bake in the new vertex positions for now.
		const int32 NumV = ROIPositionBuffer.Num();
		ParallelFor(NumV, [&](int32 k)
		{
			int VertIdx = VertexROI[k];
			const FVector3d& NewPos = ROIPositionBuffer[k];
			Mesh->SetVertex(VertIdx, NewPos, false);
		});

		// compute all the mirror vertex positions
		Symmetry->ComputeSymmetryConstrainedPositions(VertexROI, SymmetricVertexROI, ROIPositionBuffer, SymmetricROIPositionBuffer);
	}

	// once stamp is applied, we can start updating vertex change, which can happen async as we saved all necessary info
	TFuture<void> SaveVertexFuture;
	if (ActiveVertexChange != nullptr)
	{
		SaveVertexFuture = Async(VertexSculptToolAsyncExecTarget, [this]()
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_SyncMeshWithPositionBuffer_UpdateChange);
			const int32 NumV = ROIPositionBuffer.Num();
			for (int k = 0; k < NumV; ++k)
			{
				int VertIdx = VertexROI[k];
				ActiveVertexChange->UpdateVertex(VertIdx, ROIPrevPositionBuffer[k], ROIPositionBuffer[k]);
			}

			if (bApplySymmetry)
			{
				int32 NumSymV = SymmetricVertexROI.Num();
				for (int32 k = 0; k < NumSymV; ++k)
				{
					if (SymmetricVertexROI[k] >= 0)
					{
						ActiveVertexChange->UpdateVertex(SymmetricVertexROI[k], SymmetricROIPrevPositionBuffer[k], SymmetricROIPositionBuffer[k]);
					}
				}
			}

		});
	}

	// now actually update the mesh, which happens on the game thread
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_ApplyStamp_Sync);
		const int32 NumV = ROIPositionBuffer.Num();

		// If we are applying symmetry, we already baked these positions in in the branch above and
		// can skip it now, otherwise we update the mesh (todo: profile ParallelFor here, is it helping or hurting?)
		if (bApplySymmetry == false)
		{
			ParallelFor(NumV, [&](int32 k)
			{
				int VertIdx = VertexROI[k];
				const FVector3d& NewPos = ROIPositionBuffer[k];
				Mesh->SetVertex(VertIdx, NewPos, false);
			});
		}

		// if applying symmetry, bake in new symmetric positions
		if (bApplySymmetry)
		{
			ParallelFor(NumV, [&](int32 k)
			{
				int VertIdx = SymmetricVertexROI[k];
				if (Mesh->IsVertex(VertIdx))
				{
					const FVector3d& NewPos = SymmetricROIPositionBuffer[k];
					Mesh->SetVertex(VertIdx, NewPos, false);
				}
			});
		}

		Mesh->UpdateChangeStamps(true, false);
	}

	LastStamp = CurrentStamp;
	LastStamp.TimeStamp = FDateTime::Now();

	// let caller wait for this to finish
	return SaveVertexFuture;
}




bool UMorphTargetCreator::IsHitTriangleBackFacing(int32 TriangleID, const FDynamicMesh3* QueryMesh)
{
	if(!bHasToolStarted && !TargetActor) return false;

	if (TriangleID != IndexConstants::InvalidID)
	{
		FViewCameraState StateOut;
		GetToolManager()->GetContextQueriesAPI()->GetCurrentViewState(StateOut);
		FVector3d LocalEyePosition(CurTargetTransform.InverseTransformPosition((FVector3d)StateOut.Position));

		FVector3d Normal, Centroid;
		double Area;
		QueryMesh->GetTriInfo(TriangleID, Normal, Area, Centroid);

		return (Normal.Dot((Centroid - LocalEyePosition)) >= 0);
	}
	return false;
}


int32 UMorphTargetCreator::FindHitSculptMeshTriangle(const FRay3d& LocalRay)
{
	if(!bHasToolStarted && !TargetActor) return INDEX_NONE;

	// need this to finish before we can touch Octree
	WaitForPendingStampUpdate();

	int32 HitTID = Octree.FindNearestHitObject(LocalRay);
	if (GetBrushCanHitBackFaces() == false && IsHitTriangleBackFacing(HitTID, GetSculptMesh()))
	{
		HitTID = IndexConstants::InvalidID;
	}
	return HitTID;
}

int32 UMorphTargetCreator::FindHitTargetMeshTriangle(const FRay3d& LocalRay)
{
	if(!bHasToolStarted && !TargetActor) return INDEX_NONE;

	int32 HitTID = BaseMeshSpatial.FindNearestHitObject(LocalRay);
	if (GetBrushCanHitBackFaces() == false && IsHitTriangleBackFacing(HitTID, GetBaseMesh()))
	{
		HitTID = IndexConstants::InvalidID;
	}
	return HitTID;
}



bool UMorphTargetCreator::UpdateBrushPosition(const FRay& WorldRay)
{
	if(!bHasToolStarted && !TargetActor) return false;

	TUniquePtr<FHandyManMeshSculptBrushOp>& UseBrushOp = GetActiveBrushOp();

	bool bHit = false; 
	EHandyManSculptBrushOpTargetType TargetType = UseBrushOp->GetBrushTargetType();
	switch (TargetType)
	{
	case EHandyManSculptBrushOpTargetType::SculptMesh:
		bHit = UpdateBrushPositionOnSculptMesh(WorldRay, false);
		break;
	case EHandyManSculptBrushOpTargetType::TargetMesh:
		bHit = UpdateBrushPositionOnTargetMesh(WorldRay, false);
		break;
	case EHandyManSculptBrushOpTargetType::ActivePlane:
		//UpdateBrushPositionOnActivePlane(WorldRay);
		bHit = UpdateBrushPositionOnSculptMesh(WorldRay, false);
		break;
	}

	if (bHit && UseBrushOp->GetAlignStampToView())
	{
		AlignBrushToView();
	}

	return bHit;
}




void UMorphTargetCreator::UpdateHoverStamp(const FFrame3d& StampFrameWorld)
{
	if(!bHasToolStarted && !TargetActor) return;
	
	FFrame3d HoverFrame = StampFrameWorld;
	if (bHaveBrushAlpha && (AlphaProperties->RotationAngle != 0))
	{
		HoverFrame.Rotate(FQuaterniond(HoverFrame.Z(), AlphaProperties->RotationAngle, true));
	}
	Super::UpdateHoverStamp(HoverFrame);
}

bool UMorphTargetCreator::CanAccept() const
{
	return TargetActor != nullptr && TargetActor->GetMorphTargetMeshMap().Num() > 0;
}

bool UMorphTargetCreator::HitTest(const FRay& Ray, FHitResult& OutHit)
{
	return TargetActor && TargetActor->GetMorphTargetMeshMap().Num() > 0 && Super::HitTest(Ray, OutHit);
}

FInputRayHit UMorphTargetCreator::TestIfCanBeginClickDrag_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
	FHitResult OutHit;
	if (HitTest(ClickPos.WorldRay, OutHit) && bHasToolStarted)
	{
		return FInputRayHit(OutHit.Distance);
	}
	
	return FInputRayHit();
}

bool UMorphTargetCreator::OnUpdateHover(const FInputDeviceRay& DevicePos)
{
	if(!bHasToolStarted && !TargetActor) return false;
	// 4.26 HOTFIX: update LastWorldRay position so that we have it for updating WorkPlane position
	LastWorldRay = DevicePos.WorldRay;

	PendingStampType = SculptProperties->PrimaryBrushType;
	if(ensure(InStroke() == false))
	{
		UpdateBrushPosition(DevicePos.WorldRay);

		if (BrushIndicatorMaterial)
		{
			BrushIndicatorMaterial->SetScalarParameterValue(TEXT("FalloffRatio"), GetCurrentBrushFalloff());

			switch (SculptProperties->PrimaryFalloffType)
			{
			default:
			case EHandyManMeshSculptFalloffType::Smooth:
			case EHandyManMeshSculptFalloffType::BoxSmooth:
				BrushIndicatorMaterial->SetScalarParameterValue(TEXT("FalloffMode"), 0.0f);
				break;
			case EHandyManMeshSculptFalloffType::Linear:
			case EHandyManMeshSculptFalloffType::BoxLinear:
				BrushIndicatorMaterial->SetScalarParameterValue(TEXT("FalloffMode"), 0.3333333f);
				break;
			case EHandyManMeshSculptFalloffType::Inverse:
			case EHandyManMeshSculptFalloffType::BoxInverse:
				BrushIndicatorMaterial->SetScalarParameterValue(TEXT("FalloffMode"), 0.6666666f);
				break;
			case EHandyManMeshSculptFalloffType::Round:
			case EHandyManMeshSculptFalloffType::BoxRound:
				BrushIndicatorMaterial->SetScalarParameterValue(TEXT("FalloffMode"), 1.0f);
				break;
			}

			switch (SculptProperties->PrimaryFalloffType)
			{
			default:
			case EHandyManMeshSculptFalloffType::Smooth:
			case EHandyManMeshSculptFalloffType::Linear:
			case EHandyManMeshSculptFalloffType::Inverse:
			case EHandyManMeshSculptFalloffType::Round:
				BrushIndicatorMaterial->SetScalarParameterValue(TEXT("FalloffShape"), 0.0f);
				break;
			case EHandyManMeshSculptFalloffType::BoxSmooth:
			case EHandyManMeshSculptFalloffType::BoxLinear:
			case EHandyManMeshSculptFalloffType::BoxInverse:
			case EHandyManMeshSculptFalloffType::BoxRound:
				BrushIndicatorMaterial->SetScalarParameterValue(TEXT("FalloffShape"), 1.0f);
			}
			
		}
	}

	return true;
}


void UMorphTargetCreator::Render(IToolsContextRenderAPI* RenderAPI)
{
	if(!bHasToolStarted && !TargetActor) return;
	
	Super::Render(RenderAPI);

	// draw a dot for the symmetric brush stamp position
	if (bApplySymmetry)
	{
		FToolDataVisualizer Visualizer;
		Visualizer.BeginFrame(RenderAPI);
		FVector3d MirrorPoint = CurTargetTransform.TransformPosition(
			Symmetry->GetMirroredPosition(HoverStamp.LocalFrame.Origin));
		Visualizer.DrawPoint(MirrorPoint, FLinearColor(1.0, 0.1, 0.1, 1), 5.0f, false);
		Visualizer.EndFrame();
	}
}


void UMorphTargetCreator::OnTick(float DeltaTime)
{
	if(!bHasToolStarted && !TargetActor) return;
	
	Super::OnTick(DeltaTime);

	TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_Tick);

	// process the undo update
	if (bUndoUpdatePending)
	{
		// wait for updates
		WaitForPendingUndoRedo();

		// post rendering update
		DynamicMeshComponent->FastNotifyTriangleVerticesUpdated(AccumulatedTriangleROI,
			EMeshRenderAttributeFlags::Positions | EMeshRenderAttributeFlags::VertexNormals);
		GetToolManager()->PostInvalidation();

		// ignore stamp and wait for next tick to do anything else
		bUndoUpdatePending = false;
		return;
	}

	// if user changed to not-frozen, we need to reinitialize the target
	if (bCachedFreezeTarget != SculptProperties->bFreezeTarget)
	{
		UpdateBaseMesh(nullptr);
		bTargetDirty = false;
	}

	if (InStroke())
	{
		//UE_LOG(LogTemp, Warning, TEXT("dt is %.3f, tick fps %.2f - roi size %d/%d"), DeltaTime, 1.0 / DeltaTime, VertexROI.Num(), TriangleROI.Num());
		TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_Tick_StrokeUpdate);
		FDynamicMesh3* Mesh = GetSculptMesh();

		// update brush position
		if (UpdateStampPosition(GetPendingStampRayWorld()) == false)
		{
			return;
		}
		UpdateStampPendingState();
		if (IsStampPending() == false)
		{
			return;
		}

		// need to make sure previous stamp finished
		WaitForPendingStampUpdate();

		// update sculpt ROI
		UpdateROI(CurrentStamp.LocalFrame.Origin);

		// Append updated ROI to modified region (async). For some reason this is very expensive,
		// maybe because of TSet? but we have a lot of time to do it.
		TFuture<void> AccumulateROI = Async(VertexSculptToolAsyncExecTarget, [this]()
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_Tick_AccumROI);
			for (int32 tid : TriangleROIArray)
			{
				AccumulatedTriangleROI.Add(tid);
			}
		});

		// Start precomputing the normals ROI. This is currently the most expensive single thing we do next
		// to Octree re-insertion, despite it being almost trivial. Why?!?
		bool bUsingOverlayNormalsOut = false;
		TFuture<void> NormalsROI = Async(VertexSculptToolAsyncExecTarget, [Mesh, &bUsingOverlayNormalsOut, this]()
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_Tick_NormalsROI);

			//HandyMan::SculptUtil::PrecalculateNormalsROI(Mesh, TriangleROIArray, NormalsROIBuilder, bUsingOverlayNormals, false);
			HandyMan::SculptUtil::PrecalculateNormalsROI(Mesh, TriangleROIArray, NormalsFlags, bUsingOverlayNormalsOut, false);
		});

		// NOTE: you might try to speculatively do the octree remove here, to save doing it later on Reinsert().
		// This will not improve things, as Reinsert() checks if it needs to actually re-insert, which avoids many
		// removes, and does much of the work of Remove anyway.

		// Apply the stamp. This will return a future that is updating the vertex-change record, 
		// which can run until the end of the frame, as it is using cached information
		TFuture<void> UpdateChangeFuture = ApplyStamp();

		// begin octree rebuild calculation
		StampUpdateOctreeFuture = Async(VertexSculptToolAsyncExecTarget, [this]()
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_Tick_OctreeReinsert);
			Octree.ReinsertTrianglesParallel(TriangleROIArray, OctreeUpdateTempBuffer, OctreeUpdateTempFlagBuffer);
		});
		bStampUpdatePending = true;
		//TFuture<void> OctreeRebuild = Async(VertexSculptToolAsyncExecTarget, [&]()
		//{
		//	TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_Tick_OctreeReinsert);
		//	Octree.ReinsertTriangles(TriangleROIArray);
		//});

		// TODO: first step of RecalculateROINormals() is to convert TriangleROI into vertex or element ROI.
		// We can do this while we are computing stamp!

		// precompute dynamic mesh update info
		TArray<int32> RenderUpdateSets; FAxisAlignedBox3d RenderUpdateBounds;
		TFuture<bool> RenderUpdatePrecompute = DynamicMeshComponent->FastNotifyTriangleVerticesUpdated_TryPrecompute(
				TriangleROIArray, RenderUpdateSets, RenderUpdateBounds);

		// recalculate normals. This has to complete before we can update component
		// (in fact we could do it per-chunk...)
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_Tick_RecalcNormals);
			NormalsROI.Wait();
			HandyMan::SculptUtil::RecalculateROINormals(Mesh, NormalsFlags, bUsingOverlayNormalsOut);
			//HandyMan::SculptUtil::RecalculateROINormals(Mesh, NormalsROIBuilder.Indices(), bUsingOverlayNormals);
		}

		{
			TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_Tick_UpdateMesh);
			RenderUpdatePrecompute.Wait();
			DynamicMeshComponent->FastNotifyTriangleVerticesUpdated_ApplyPrecompute(TriangleROIArray,
				EMeshRenderAttributeFlags::Positions | EMeshRenderAttributeFlags::VertexNormals,
				RenderUpdatePrecompute, RenderUpdateSets, RenderUpdateBounds);

			GetToolManager()->PostInvalidation();
		}

		// we don't really need to wait for these to happen to end Tick()...
		UpdateChangeFuture.Wait();
		AccumulateROI.Wait();
	} 
	else if (bTargetDirty)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_Tick_UpdateTarget);
		check(InStroke() == false);

		// this spawns futures that we could allow to run while other things happen...
		UpdateBaseMesh(&AccumulatedTriangleROI);
		AccumulatedTriangleROI.Reset();

		bTargetDirty = false;
	}

}



void UMorphTargetCreator::WaitForPendingStampUpdate()
{
	if (bStampUpdatePending)
	{
		StampUpdateOctreeFuture.Wait();
		bStampUpdatePending = true;
	}
}



void UMorphTargetCreator::UpdateBaseMesh(const TSet<int32>* TriangleSet)
{
	if (SculptProperties != nullptr)
	{
		bCachedFreezeTarget = SculptProperties->bFreezeTarget;
		if (SculptProperties->bFreezeTarget)
		{
			return;   // do not update frozen target
		}
	}

	const FDynamicMesh3* SculptMesh = GetSculptMesh();
	if ( ! TriangleSet )
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_Target_FullUpdate);
		BaseMesh.Copy(*SculptMesh, false, false, false, false);
		BaseMesh.EnableVertexNormals(FVector3f::UnitZ());
		FMeshNormals::QuickComputeVertexNormals(BaseMesh);
		BaseMeshSpatial.SetMaxTreeDepth(8);
		BaseMeshSpatial = FDynamicMeshOctree3();   // need to clear...
		BaseMeshSpatial.Initialize(&BaseMesh);
	}
	else
	{
		BaseMeshIndexBuffer.Reset();
		for ( int32 tid : *TriangleSet)
		{ 
			FIndex3i Tri = BaseMesh.GetTriangle(tid);
			BaseMesh.SetVertex(Tri.A, SculptMesh->GetVertex(Tri.A));
			BaseMesh.SetVertex(Tri.B, SculptMesh->GetVertex(Tri.B));
			BaseMesh.SetVertex(Tri.C, SculptMesh->GetVertex(Tri.C));
			BaseMeshIndexBuffer.Add(tid);
		}
		auto UpdateBaseNormals = Async(VertexSculptToolAsyncExecTarget, [this]()
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_Target_UpdateBaseNormals);
			FMeshNormals::QuickComputeVertexNormalsForTriangles(BaseMesh, BaseMeshIndexBuffer);
		});
		auto ReinsertTriangles = Async(VertexSculptToolAsyncExecTarget, [TriangleSet, this]()
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(UMorphTargetCreator_Target_Reinsert);
			BaseMeshSpatial.ReinsertTriangles(*TriangleSet);
		});
		UpdateBaseNormals.Wait();
		ReinsertTriangles.Wait();
	}
}


bool UMorphTargetCreator::GetBaseMeshNearest(int32 VertexID, const FVector3d& Position, double SearchRadius, FVector3d& TargetPosOut, FVector3d& TargetNormalOut)
{
	TargetPosOut = BaseMesh.GetVertex(VertexID);
	TargetNormalOut = (FVector3d)BaseMesh.GetVertexNormal(VertexID);
	return true;
}






void UMorphTargetCreator::IncreaseBrushSpeedAction()
{
	TUniquePtr<FHandyManMeshSculptBrushOp>& UseBrushOp = GetActiveBrushOp();
	float CurStrength = UseBrushOp->PropertySet->GetStrength();
	float NewStrength = FMath::Clamp(CurStrength + 0.05f, 0.0f, 1.0f);
	UseBrushOp->PropertySet->SetStrength(NewStrength);
	NotifyOfPropertyChangeByTool(UseBrushOp->PropertySet.Get());
}

void UMorphTargetCreator::DecreaseBrushSpeedAction()
{
	TUniquePtr<FHandyManMeshSculptBrushOp>& UseBrushOp = GetActiveBrushOp();
	float CurStrength = UseBrushOp->PropertySet->GetStrength();
	float NewStrength = FMath::Clamp(CurStrength - 0.05f, 0.0f, 1.0f);
	UseBrushOp->PropertySet->SetStrength(NewStrength);
	NotifyOfPropertyChangeByTool(UseBrushOp->PropertySet.Get());
}

void UMorphTargetCreator::UpdateBrushAlpha(UTexture2D* NewAlpha)
{
	if (this->BrushAlpha != NewAlpha)
	{
		this->BrushAlpha = NewAlpha;
		if (this->BrushAlpha != nullptr)
		{
			TImageBuilder<FVector4f> AlphaValues;

			constexpr bool bPreferPlatformData = false;
			const bool bReadOK = UE::AssetUtils::ReadTexture(this->BrushAlpha, AlphaValues, bPreferPlatformData);
			if (bReadOK)
			{
				BrushAlphaValues = MoveTemp(AlphaValues);
				BrushAlphaDimensions = AlphaValues.GetDimensions();
				bHaveBrushAlpha = true;

				BrushIndicatorMaterial->SetTextureParameterValue(TEXT("BrushAlpha"), NewAlpha);
				BrushIndicatorMaterial->SetScalarParameterValue(TEXT("AlphaPower"), 1.0);

				return;
			}
		}
		bHaveBrushAlpha = false;
		BrushAlphaValues = TImageBuilder<FVector4f>();
		BrushAlphaDimensions = FImageDimensions();

		BrushIndicatorMaterial->SetTextureParameterValue(TEXT("BrushAlpha"), nullptr);
		BrushIndicatorMaterial->SetScalarParameterValue(TEXT("AlphaPower"), 0.0);
	}
}


double UMorphTargetCreator::SampleBrushAlpha(const FHandyManSculptBrushStamp& Stamp, const FVector3d& Position) const
{
	if (! bHaveBrushAlpha) return 1.0;

	static const FVector4f InvalidValue(0, 0, 0, 0);

	FVector2d AlphaUV = Stamp.LocalFrame.ToPlaneUV(Position, 2);
	double u = AlphaUV.X / Stamp.Radius;
	u = 1.0 - (u + 1.0) / 2.0;
	double v = AlphaUV.Y / Stamp.Radius;
	v = 1.0 - (v + 1.0) / 2.0;
	if (u < 0 || u > 1) return 0.0;
	if (v < 0 || v > 1) return 0.0;
	FVector4f AlphaValue = BrushAlphaValues.BilinearSampleUV<float>(FVector2d(u, v), InvalidValue);
	return FMathd::Clamp(AlphaValue.X, 0.0, 1.0);
}


void UMorphTargetCreator::TryToInitializeSymmetry()
{
	// Attempt to find symmetry, favoring the X axis, then Y axis, if a single symmetry plane is not immediately found
	// Uses local mesh surface (angle sum, normal) to help disambiguate final matches, but does not require exact topology matches across the plane

	FAxisAlignedBox3d Bounds = GetSculptMesh()->GetBounds(true);

	TArray<FVector3d> PreferAxes;
	PreferAxes.Add(this->InitialTargetTransform.GetRotation().AxisX());
	PreferAxes.Add(this->InitialTargetTransform.GetRotation().AxisY());

	FMeshPlanarSymmetry FindSymmetry;
	FFrame3d FoundPlane;
	if (FindSymmetry.FindPlaneAndInitialize(GetSculptMesh(), Bounds, FoundPlane, PreferAxes))
	{
		Symmetry = MakePimpl<FMeshPlanarSymmetry>();
		*Symmetry = MoveTemp(FindSymmetry);
		bMeshSymmetryIsValid = true;

		if (SymmetryProperties)
		{
			bApplySymmetry = bMeshSymmetryIsValid && SymmetryProperties->bEnableSymmetry;
		}
	}
}


//
// Change Tracking
//
void UMorphTargetCreator::BeginChange()
{
	check(ActiveVertexChange == nullptr);
	ActiveVertexChange = new FMeshVertexChangeBuilder();
	LongTransactions.Open(LOCTEXT("MorphTargetSculptChange", "Brush Stroke"), GetToolManager());
}

void UMorphTargetCreator::EndChange()
{
	check(ActiveVertexChange);

	TUniquePtr<TWrappedToolCommandChange<FMeshVertexChange>> NewChange = MakeUnique<TWrappedToolCommandChange<FMeshVertexChange>>();
	NewChange->WrappedChange = MoveTemp(ActiveVertexChange->Change);
	NewChange->BeforeModify = [this](bool bRevert)
	{
		this->WaitForPendingUndoRedo();
	};

	GetToolManager()->EmitObjectChange(DynamicMeshComponent, MoveTemp(NewChange), LOCTEXT("MorphTargetSculptChange", "Brush Stroke"));
	if (bMeshSymmetryIsValid && bApplySymmetry == false)
	{
		// if we end a stroke while symmetry is possible but disabled, we now have to assume that symmetry is no longer possible
		GetToolManager()->EmitObjectChange(this, MakeUnique<FMorphTargetNonSymmetricChange>(), LOCTEXT("DisableSymmetryChange", "Disable Symmetry"));
		bMeshSymmetryIsValid = false;
		SymmetryProperties->bSymmetryCanBeEnabled = bMeshSymmetryIsValid;
	}
	LongTransactions.Close(GetToolManager());

	delete ActiveVertexChange;
	ActiveVertexChange = nullptr;
}


void UMorphTargetCreator::WaitForPendingUndoRedo()
{
	if (bUndoUpdatePending)
	{
		UndoNormalsFuture.Wait();
		UndoUpdateOctreeFuture.Wait();
		UndoUpdateBaseMeshFuture.Wait();
		bUndoUpdatePending = false;
	}
}

void UMorphTargetCreator::OnDynamicMeshComponentChanged(UDynamicMeshComponent* Component, const FMeshVertexChange* Change, bool bRevert)
{
	// have to wait for any outstanding stamp update to finish...
	WaitForPendingStampUpdate();
	// wait for previous Undo to finish (possibly never hit because the change records do it?)
	WaitForPendingUndoRedo();

	FDynamicMesh3* Mesh = GetSculptMesh();

	// figure out the set of modified triangles
	AccumulatedTriangleROI.Reset();
	UE::Geometry::VertexToTriangleOneRing(Mesh, Change->Vertices, AccumulatedTriangleROI);

	// start the normal recomputation
	UndoNormalsFuture = Async(VertexSculptToolAsyncExecTarget, [this, Mesh]()
	{
		HandyMan::SculptUtil::RecalculateROINormals(Mesh, AccumulatedTriangleROI, NormalsROIBuilder);
		return true;
	});

	// start the octree update
	UndoUpdateOctreeFuture = Async(VertexSculptToolAsyncExecTarget, [this, Mesh]()
	{
		Octree.ReinsertTriangles(AccumulatedTriangleROI);
		return true;
	});

	// start the base mesh update
	UndoUpdateBaseMeshFuture = Async(VertexSculptToolAsyncExecTarget, [this, Mesh]()
	{
		UpdateBaseMesh(&AccumulatedTriangleROI);
		return true;
	});

	// note that we have a pending update
	bUndoUpdatePending = true;
}






void UMorphTargetCreator::UpdateBrushType(EHandyManBrushType BrushType)
{
	static const FText BaseMessage = LOCTEXT("OnStartSculptTool", "Hold Shift to Smooth, Ctrl to Invert (where applicable). [/] and S/D change Size (+Shift to small-step), W/E changes Strength.");
	FTextBuilder Builder;
	Builder.AppendLine(BaseMessage);

	SetActivePrimaryBrushType((int32)BrushType);

	SetToolPropertySourceEnabled(GizmoProperties, false);
	if (BrushType == EHandyManBrushType::FixedPlane)
	{
		Builder.AppendLine(LOCTEXT("FixedPlaneTip", "Use T to reposition Work Plane at cursor, Shift+T to align to Normal, Ctrl+Shift+T to align to View"));
		SetToolPropertySourceEnabled(GizmoProperties, true);
	}


	bool bEnableAlpha = false;
	switch (BrushType)
	{
	case EHandyManBrushType::Offset:
	case EHandyManBrushType::SculptView:
	case EHandyManBrushType::SculptMax:
		bEnableAlpha = true; break;
	}
	SetToolPropertySourceEnabled(AlphaProperties, bEnableAlpha);


	GetToolManager()->DisplayMessage(Builder.ToText(), EToolMessageLevel::UserNotification);
}



void UMorphTargetCreator::UndoRedo_RestoreSymmetryPossibleState(bool bSetToValue)
{
	bMeshSymmetryIsValid = bSetToValue;
	SymmetryProperties->bSymmetryCanBeEnabled = bMeshSymmetryIsValid;
}





void FMorphTargetNonSymmetricChange::Apply(UObject* Object)
{
	if (Cast<UMorphTargetCreator>(Object))
	{
		Cast<UMorphTargetCreator>(Object)->UndoRedo_RestoreSymmetryPossibleState(false);
	}
}
void FMorphTargetNonSymmetricChange::Revert(UObject* Object)
{
	if (Cast<UMorphTargetCreator>(Object))
	{
		Cast<UMorphTargetCreator>(Object)->UndoRedo_RestoreSymmetryPossibleState(true);
	}
}




#undef LOCTEXT_NAMESPACE
