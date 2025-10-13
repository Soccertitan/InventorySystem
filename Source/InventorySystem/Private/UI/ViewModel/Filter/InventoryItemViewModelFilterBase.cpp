// Copyright Soccertitan


#include "UI/ViewModel/Filter/InventoryItemViewModelFilterBase.h"

#include "BlueprintNodeHelpers.h"
#include "InventoryFastTypes.h"
#include "UI/ViewModel/ItemViewModel.h"

UInventoryItemViewModelFilterBase::UInventoryItemViewModelFilterBase()
{
	bHasShouldBeginFilter = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_ShouldBeginFilter"), *this, *StaticClass());
	bHasDoesItemViewModelPassFilter = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_DoesItemViewModelPassFilter"), *this, *StaticClass());
}

void UInventoryItemViewModelFilterBase::FilterItemViewModels(const UObject* Context, TArray<UItemViewModel*>& ItemViewModels) const
{
	if (ShouldBeginFilter(Context, ItemViewModels))
	{
		for (int32 idx = ItemViewModels.Num() - 1; idx >= 0; idx--)
		{
			if (ItemViewModels[idx] && DoesItemViewModelPassFilter(Context, ItemViewModels[idx]))
			{
				continue;
			}
			ItemViewModels.RemoveAt(idx);
		}
	}
}

bool UInventoryItemViewModelFilterBase::ShouldBeginFilter(const UObject* Context, TArray<UItemViewModel*>& ItemViewModels) const
{
	if (bHasShouldBeginFilter)
	{
		return K2_ShouldBeginFilter(Context, ItemViewModels);
	}
	return true;
}

bool UInventoryItemViewModelFilterBase::DoesItemViewModelPassFilter(const UObject* Context, UItemViewModel* ItemViewModel) const
{
	if (!ItemViewModel->GetItem().IsValid())
	{
		return false;
	}

	if (bHasDoesItemViewModelPassFilter)
	{
		return K2_DoesItemViewModelPassFilter(Context, ItemViewModel);
	}

	return true;
}


