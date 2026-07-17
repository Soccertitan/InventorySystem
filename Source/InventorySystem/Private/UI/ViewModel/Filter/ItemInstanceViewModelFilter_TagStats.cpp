// Copyright Soccertitan 2025


#include "UI/ViewModel/Filter/ItemInstanceViewModelFilter_TagStats.h"

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

bool UItemInstanceViewModelFilter_TagStats::ShouldFilterItemInstance(const UObject* Context, UItemInstanceViewModel* ItemInstanceViewModel) const
{
	if (Super::ShouldFilterItemInstance(Context, ItemInstanceViewModel))
	{
		return true;
	}

	if (const FItem* ItemPtr = ItemInstanceViewModel->GetItem().GetPtr<FItem>())
	{
		for (const auto& Requirement : RequiredTagStats.GetItems())
		{
			int32 CurrentValue = ItemPtr->GameplayTagStackContainer.GetStackCount(Requirement.GetTag());
			if (CurrentValue <= Requirement.GetCount())
			{
				return true;
			}
		}
		return false;
	}

	return true;
}
