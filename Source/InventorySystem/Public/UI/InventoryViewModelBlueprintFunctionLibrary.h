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
	/** Creates an ItemInstance ViewModel from the Item's UI fragment. If it does not exist, uses the base ItemInstanceViewModel class. */
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance", meta = (DefaultToSelf = "Owner"))
	static UItemInstanceViewModel* CreateItemInstanceViewModel(UObject* Owner, const FItemInstance& ItemInstance);
	
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	static void SortItemInstanceViewModels(const UItemInstanceViewModelSortingPreset* SortingPreset, UPARAM(ref) TArray<UItemInstanceViewModel*>& InViewModels);
};
