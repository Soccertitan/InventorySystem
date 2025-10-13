// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "ItemContainerRule.h"
#include "Item/Item.h"
#include "ItemContainerRule_AddItem.generated.h"

/**
 * Determines if an Item is allowed to be added to the ItemContainer.
 */
UCLASS(Abstract, BlueprintType)
class INVENTORYSYSTEM_API UItemContainerRule_AddItem : public UItemContainerRule
{
	GENERATED_BODY()

public:
	UItemContainerRule_AddItem();
	
	/** Returns true if the item is allowed in the ItemContainer. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Rule")
	virtual bool CanAddItem(const TInstancedStruct<FItem>& Item, FGameplayTag& OutError) const;

protected:
	/** Returns true if the item is allowed in the ItemContainer. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "CanAddItem")
	bool K2_CanAddItem(const TInstancedStruct<FItem>& Item, FGameplayTag& OutError) const;

private:
	uint8 bHasCanAddItem : 1;
};
