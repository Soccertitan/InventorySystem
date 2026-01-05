// Copyright Soccertitan 2025


#include "ItemContainer/Rules/ItemContainerRule_ItemQuantity.h"

#include "BlueprintNodeHelpers.h"
#include "Item/Item.h"

UItemContainerRule_ItemQuantity::UItemContainerRule_ItemQuantity()
{
	bHasGetItemQuantityLimit = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_GetItemLimit"), *this, *StaticClass());
	bHasGetItemContainerLimit = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_GetItemContainerLimit"), *this, *StaticClass());
	bHasGetInventoryManagerLimit = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_GetInventoryManagerLimit"), *this, *StaticClass());
}

int32 UItemContainerRule_ItemQuantity::GetItemLimit(const TInstancedStruct<FItem>& Item) const
{
	if (bHasGetItemQuantityLimit)
	{
		return K2_GetItemLimit(Item);
	}
	return MAX_int32;
}

int32 UItemContainerRule_ItemQuantity::GetItemContainerLimit(const TInstancedStruct<FItem>& Item) const
{
	if (bHasGetItemContainerLimit)
	{
		return K2_GetItemContainerLimit(Item);
	}
	return MAX_int32;
}

int32 UItemContainerRule_ItemQuantity::GetInventoryManagerLimit(const TInstancedStruct<FItem>& Item) const
{
	if (bHasGetInventoryManagerLimit)
	{
		return K2_GetInventoryManagerLimit(Item);
	}
	return MAX_int32;
}
