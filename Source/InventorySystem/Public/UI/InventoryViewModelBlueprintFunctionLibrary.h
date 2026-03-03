// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
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
	UFUNCTION(BlueprintCallable, Category = "Inventory System|View Model", meta = (DefaultToSelf = "Owner"))
	static UItemInstanceViewModel* CreateItemInstanceViewModel(UObject* Owner, const FItemInstance& ItemInstance);
};
