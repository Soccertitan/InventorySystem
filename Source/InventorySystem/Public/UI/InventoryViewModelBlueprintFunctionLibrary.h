// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryViewModelBlueprintFunctionLibrary.generated.h"

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
	
	UFUNCTION(BlueprintPure, Category = "Viewmodel|ItemInstance")
	static bool DoesItemHaveUIFragment(const TInstancedStruct<FItem>& Item);
};
