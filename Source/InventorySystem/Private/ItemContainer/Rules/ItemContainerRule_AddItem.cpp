// Copyright Soccertitan 2025


#include "ItemContainer/Rules/ItemContainerRule_AddItem.h"

#include "BlueprintNodeHelpers.h"

UItemContainerRule_AddItem::UItemContainerRule_AddItem()
{
	bHasCanAddItem = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_CanAddItem"), *this, *StaticClass());
}

bool UItemContainerRule_AddItem::CanAddItem(const TInstancedStruct<FItem>& Item, FGameplayTag& OutError) const
{
	if (bHasCanAddItem)
	{
		return K2_CanAddItem(Item, OutError);
	}
	return true;
}
