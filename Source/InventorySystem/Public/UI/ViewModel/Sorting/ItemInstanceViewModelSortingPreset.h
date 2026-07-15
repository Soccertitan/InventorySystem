// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemInstanceViewModelSortingPreset.generated.h"

class UItemInstanceViewModelSortingAlgorithm;

/**
 * An asset that contains a collection of Sorting functions that are processed top to bottom.
 */
UCLASS(Const)
class INVENTORYSYSTEM_API UItemInstanceViewModelSortingPreset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// Can be shown in the UI for the type of sort this preset is.
	UPROPERTY(EditAnywhere)
	FText DisplayText;
	
	// The sorting will occur from top to bottom in the stack.
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UItemInstanceViewModelSortingAlgorithm>> SortingAlgorithms;
};
