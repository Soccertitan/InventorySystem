// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryViewModelBlueprintFunctionLibrary.generated.h"

class UItemInstanceViewModelSortingPreset;
struct FItemInstance;
class UItemInstanceViewModel;

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryViewModelBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	static void StableSortItemInstanceViewModels(const UObject* Context, const UItemInstanceViewModelSortingPreset* SortingPreset, UPARAM(ref) TArray<UItemInstanceViewModel*>& InViewModels);
};
