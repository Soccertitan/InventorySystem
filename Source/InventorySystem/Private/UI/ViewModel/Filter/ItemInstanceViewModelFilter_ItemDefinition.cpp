// Copyright Soccertitan 2025


#include "UI/ViewModel/Filter/ItemInstanceViewModelFilter_ItemDefinition.h"

#include "InventoryFastTypes.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"


bool UItemInstanceViewModelFilter_ItemDefinition::ShouldBeginFilter(const UObject* Context, TArray<UItemInstanceViewModel*>& ItemInstanceViewModels) const
{
	if (!Super::ShouldBeginFilter(Context, ItemInstanceViewModels))
	{
		return false;
	}

	if (ItemDefinitions.IsEmpty())
	{
		return false;
	}

	return true;
}

bool UItemInstanceViewModelFilter_ItemDefinition::ShouldFilterItemInstance(const UObject* Context, UItemInstanceViewModel* ItemInstanceViewModel) const
{
	if (Super::ShouldFilterItemInstance(Context, ItemInstanceViewModel))
	{
		return true;
	}

	for (const TSoftObjectPtr<UItemDefinition> ItemDefinition : ItemDefinitions)
	{
		if (ItemInstanceViewModel->GetItem().Get<FItem>().GetItemDefinition() == ItemDefinition)
		{
			return false;
		}
	}

	return true;
}
