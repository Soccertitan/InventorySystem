// Copyright Soccertitan 2025


#include "UI/ViewModel/Filter/ItemInstanceViewModelFilter.h"

#include "BlueprintNodeHelpers.h"
#include "InventoryFastTypes.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"

UItemInstanceViewModelFilter::UItemInstanceViewModelFilter()
{
	bHasShouldBeginFilter = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_ShouldBeginFilter"), *this, *StaticClass());
	bHasDoesItemInstanceViewModelPassFilter = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_ShouldFilterItemInstance"), *this, *StaticClass());
}

void UItemInstanceViewModelFilter::FilterItemInstanceViewModels(const UObject* Context, TArray<UItemInstanceViewModel*>& ItemInstanceViewModels) const
{
	if (ShouldBeginFilter(Context, ItemInstanceViewModels))
	{
		for (int32 idx = ItemInstanceViewModels.Num() - 1; idx >= 0; idx--)
		{
			if (ItemInstanceViewModels[idx] && !ShouldFilterItemInstance(Context, ItemInstanceViewModels[idx]))
			{
				continue;
			}
			ItemInstanceViewModels.RemoveAt(idx);
		}
	}
}

bool UItemInstanceViewModelFilter::ShouldBeginFilter(const UObject* Context, TArray<UItemInstanceViewModel*>& ItemInstanceViewModels) const
{
	if (bHasShouldBeginFilter)
	{
		return K2_ShouldBeginFilter(Context, ItemInstanceViewModels);
	}
	return true;
}

bool UItemInstanceViewModelFilter::ShouldFilterItemInstance(const UObject* Context, UItemInstanceViewModel* ItemInstanceViewModel) const
{
	if (bHasDoesItemInstanceViewModelPassFilter)
	{
		return K2_ShouldFilterItemInstance(Context, ItemInstanceViewModel);
	}

	return false;
}


