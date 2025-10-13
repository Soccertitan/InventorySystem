// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "ItemContainer/ItemContainer.h"
#include "ItemDropItemContainer.generated.h"

/**
 * An ItemContainer that does not auto stack items and allows each item to be added as is without restriction.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class INVENTORYSYSTEM_API UItemDropItemContainer : public UItemContainer
{
	GENERATED_BODY()

public:
	UItemDropItemContainer();

	virtual int32 GetItemStackQuantityLimit(const TInstancedStruct<FItem>& Item) const override;

protected:
	virtual void GetAddItemPlan(const TInstancedStruct<FItem>& Item, FAddItemPlan& AddItemPlan) const override;
};
