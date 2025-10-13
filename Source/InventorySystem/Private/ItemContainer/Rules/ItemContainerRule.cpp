// Copyright Soccertitan


#include "ItemContainer/Rules/ItemContainerRule.h"

#include "ItemContainer/ItemContainer.h"

UItemContainer* UItemContainerRule::K2_GetItemContainer(const TSubclassOf<UItemContainer> ItemContainerClass) const
{
	if (ItemContainerClass)
	{
		return Cast<UItemContainer>(GetOuter());
	}
	return nullptr;
}
