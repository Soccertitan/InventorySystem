// Copyright Soccertitan


#include "UI/ViewModel/Filter/InventoryItemViewModelFilter_ItemDefinition.h"

#include "InventoryFastTypes.h"
#include "UI/ViewModel/ItemViewModel.h"


bool UInventoryItemViewModelFilter_ItemDefinition::ShouldBeginFilter(const UObject* Context, TArray<UItemViewModel*>& ItemViewModels) const
{
	if (!Super::ShouldBeginFilter(Context, ItemViewModels))
	{
		return false;
	}

	if (ItemDefinitions.IsEmpty())
	{
		return false;
	}

	return true;
}

bool UInventoryItemViewModelFilter_ItemDefinition::DoesItemViewModelPassFilter(const UObject* Context, UItemViewModel* ItemViewModel) const
{
	if (!Super::DoesItemViewModelPassFilter(Context, ItemViewModel))
	{
		return false;
	}

	bool bFoundItemDefinition = false;
	for (const TSoftObjectPtr<UItemDefinition> ItemDefinition : ItemDefinitions)
	{
		if (ItemViewModel->GetItem().Get<FItem>().GetItemDefinition() == ItemDefinition)
		{
			bFoundItemDefinition = true;
			break;
		}
	}

	if (!bFoundItemDefinition)
	{
		return false;
	}

	return true;
}
