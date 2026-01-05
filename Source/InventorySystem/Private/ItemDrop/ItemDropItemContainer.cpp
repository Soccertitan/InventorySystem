// Copyright Soccertitan 2025


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
	AddItemPlan.AddEntry(FAddItemPlanEntry(Item, AddItemPlan.GetAmountToGive()));
}
