// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScriptableToolSet.h"
#include "HandyManScriptableToolSet.generated.h"

/**
 * 
 */
UCLASS()
class HANDYMAN_API UHandyManScriptableToolSet : public UObject
{
	GENERATED_BODY()

public:

public:

	UHandyManScriptableToolSet();

	virtual ~UHandyManScriptableToolSet();

	/**
	* Forces the unloading of all tools loaded
	*/
	void UnloadAllTools();

	/**
	 * Find all UScriptableInteractiveTool classes in the current project.
	 * (Currently no support for filtering/etc)
	 */
	void ReinitializeScriptableTools(FPreToolsLoadedDelegate PreDelegate, FToolsLoadedDelegate PostDelegate, FToolsLoadingUpdateDelegate UpdateDelegate, FScriptableToolGroupSet* TagsToFilter = nullptr);

	/**
	 * Allow external code to process each UScriptableInteractiveTool in the current ToolSet
	 */
	void ForEachScriptableTool(
		TFunctionRef<void(UClass* ToolClass, UBaseScriptableToolBuilder* ToolBuilder)> ProcessToolFunc);

private:

	void HandleAssetCanDelete(const TArray<UObject*>& InObjectsToDelete, FCanDeleteAssetResult& OutCanDelete);

	void PostToolLoad(FToolsLoadedDelegate Delegate, TArray< FSoftObjectPath > ObjectsLoaded, TSharedPtr<FScriptableToolGroupSet> TagsToFilter);

	bool bActiveLoading = false;
	TSharedPtr<FStreamableHandle> AsyncLoadHandle;

	FDelegateHandle AssetCanDeleteHandle;

	struct FScriptableToolInfo
	{
		FString ToolPath;
		FString BuilderPath;
		TWeakObjectPtr<UClass> ToolClass = nullptr;
		TWeakObjectPtr<UScriptableInteractiveTool> ToolCDO;
		TWeakObjectPtr<UBaseScriptableToolBuilder> ToolBuilder;
	};
	TArray<FScriptableToolInfo> Tools;

	UPROPERTY()
	TArray<TObjectPtr<UBaseScriptableToolBuilder>> ToolBuilders;

	
};
