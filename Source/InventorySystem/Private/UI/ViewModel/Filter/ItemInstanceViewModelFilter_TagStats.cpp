// Copyright Soccertitan 2025


#include "UI/ViewModel/Filter/ItemInstanceViewModelFilter_TagStats.h"

#include "ItemContainer/ItemContainer.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"


bool UItemInstanceViewModelFilter_TagStats::ShouldBeginFilter(const UObject* Context, TArray<UItemInstanceViewModel*>& ItemInstanceViewModels) const
{
	if (!Super::ShouldBeginFilter(Context, ItemInstanceViewModels))
	{
		return false;
	}

	if (RequiredTagStats.GetItems().IsEmpty())
	{
		return false;
	}

	return true;
}

bool UItemInstanceViewModelFilter_TagStats::DoesItemInstanceViewModelPassFilter(const UObject* Context, UItemInstanceViewModel* ItemInstanceViewModel) const
{
	if (!Super::DoesItemInstanceViewModelPassFilter(Context, ItemInstanceViewModel))
	{
		return false;
	}

	if (const FItemInstance* ItemInstancePtr = ItemInstanceViewModel->GetItemContainer()->FindItemByGuid(ItemInstanceViewModel->GetGuid()))
	{
		if (const FItem* ItemPtr = ItemInstancePtr->GetItem().GetPtr<FItem>())
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
