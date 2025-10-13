// Copyright Soccertitan


#include "ItemContainer/Rules/ItemContainerRule_RemoveItem.h"

#include "BlueprintNodeHelpers.h"

UItemContainerRule_RemoveItem::UItemContainerRule_RemoveItem()
{
	bHasCanRemoveItem = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_CanRemoveItem"), *this, *StaticClass());
}

bool UItemContainerRule_RemoveItem::CanRemoveItem(const TInstancedStruct<FItem>& Item) const
{
	if (bHasCanRemoveItem)
	{
		return K2_CanRemoveItem(Item);
	}
	return true;
}
