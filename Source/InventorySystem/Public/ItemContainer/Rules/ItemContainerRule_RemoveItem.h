// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "ItemContainerRule.h"
#include "Item/Item.h"
#include "ItemContainerRule_RemoveItem.generated.h"

/**
 * Determines if an Item is allowed to be directly removed from the ItemContainer.
 */
UCLASS(Abstract, BlueprintType)
class INVENTORYSYSTEM_API UItemContainerRule_RemoveItem : public UItemContainerRule
{
	GENERATED_BODY()

public:
	UItemContainerRule_RemoveItem();

	/** Returns true if the item is allowed to be removed directly. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Rule")
	virtual bool CanRemoveItem(const TInstancedStruct<FItem>& Item) const;

protected:
	/** Returns true if the item is allowed to be removed directly. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "CanRemoveItem")
	bool K2_CanRemoveItem(const TInstancedStruct<FItem>& Item) const;

private:
	uint8 bHasCanRemoveItem : 1;
};
