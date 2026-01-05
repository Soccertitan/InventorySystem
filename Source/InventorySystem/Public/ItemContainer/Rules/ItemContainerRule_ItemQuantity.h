// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "ItemContainerRule.h"
#include "Item/Item.h"
#include "ItemContainerRule_ItemQuantity.generated.h"


/**
 * Describes the max quantity for an item and number of stacks in the ItemContainer.
 */
UCLASS(Abstract, BlueprintType)
class INVENTORYSYSTEM_API UItemContainerRule_ItemQuantity : public UItemContainerRule
{
	GENERATED_BODY()

public:
	UItemContainerRule_ItemQuantity();

	/** The maximum quantity of this item allowed in a single ItemInstance. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Rule")
	virtual int32 GetItemLimit(const TInstancedStruct<FItem>& Item) const;

	/** The maximum number of unique ItemInstances allowed in a single ItemContainer. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Rule")
	virtual int32 GetItemContainerLimit(const TInstancedStruct<FItem>& Item) const;

	/** The maximum number of unique ItemInstances allowed across all ItemContainers. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Rule")
	virtual int32 GetInventoryManagerLimit(const TInstancedStruct<FItem>& Item) const;

protected:
	/** The maximum quantity of this item allowed in a single ItemInstance. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "GetItemLimit")
	int32 K2_GetItemLimit(const TInstancedStruct<FItem>& Item) const;

	/** Return the maximum number of unique item instances the item can occupy in the ItemContainer. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "GetItemContainerLimit")
	int32 K2_GetItemContainerLimit(const TInstancedStruct<FItem>& Item) const;

	/** The maximum number of unique ItemInstances allowed across all ItemContainers. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "GetInventoryManagerLimit")
	int32 K2_GetInventoryManagerLimit(const TInstancedStruct<FItem>& Item) const;

private:
	uint8 bHasGetItemQuantityLimit : 1;
	uint8 bHasGetItemContainerLimit : 1;
	uint8 bHasGetInventoryManagerLimit : 1;
};
