// Copyright Soccertitan


#include "ItemDrop/ItemDropItemContainer.h"

UItemDropItemContainer::UItemDropItemContainer()
{
}

int32 UItemDropItemContainer::GetItemStackQuantityLimit(const TInstancedStruct<FItem>& Item) const
{
	return MAX_int32;
}

void UItemDropItemContainer::GetAddItemPlan(const TInstancedStruct<FItem>& Item, FAddItemPlan& AddItemPlan) const
{
	const FItem* ItemPtr = Item.GetPtr<FItem>();
	AddItemPlan.AddEntry(FAddItemPlanEntry(Item, ItemPtr->GetQuantity()));
}
