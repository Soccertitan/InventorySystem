// Copyright Soccertitan


#include "UI/ViewModel/Filter/InventoryItemViewModelFilter_TagStats.h"

#include "ItemContainer/ItemContainer.h"
#include "UI/ViewModel/ItemViewModel.h"


bool UInventoryItemViewModelFilter_TagStats::ShouldBeginFilter(const UObject* Context, TArray<UItemViewModel*>& ItemViewModels) const
{
	if (!Super::ShouldBeginFilter(Context, ItemViewModels))
	{
		return false;
	}

	if (RequiredTagStats.GetItems().IsEmpty())
	{
		return false;
	}

	return true;
}

bool UInventoryItemViewModelFilter_TagStats::DoesItemViewModelPassFilter(const UObject* Context, UItemViewModel* ItemViewModel) const
{
	if (!Super::DoesItemViewModelPassFilter(Context, ItemViewModel))
	{
		return false;
	}

	if (const FItemInstance* ItemInstancePtr = ItemViewModel->GetItemContainer()->FindItemByGuid(ItemViewModel->GetGuid()))
	{
		if (const FItem* ItemPtr = ItemInstancePtr->Item.GetPtr<FItem>())
		{
			for (const auto& Requirement : RequiredTagStats.GetItems())
			{
				int32 CurrentValue = ItemPtr->GameplayTagStackContainer.GetStackCount(Requirement.GetTag());
				if (CurrentValue <= Requirement.GetCount())
				{
					return false;
				}
			}

			return true;
		}
	}

	return false;
}
