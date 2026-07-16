// Copyright Soccertitan 2025


#include "UI/InventoryViewModelBlueprintFunctionLibrary.h"

#include "UI/ViewModel/ItemInstanceViewModel.h"
#include "UI/ViewModel/Sorting/ItemInstanceViewModelSorting.h"
#include "UI/ViewModel/Sorting/ItemInstanceViewModelSortingPreset.h"


void UInventoryViewModelBlueprintFunctionLibrary::StableSortItemInstanceViewModels(const UObject* Context, const UItemInstanceViewModelSortingPreset* SortingPreset, TArray<UItemInstanceViewModel*>& InViewModels)
{
	if (!SortingPreset || InViewModels.IsEmpty())
	{
		return;
	}

	Algo::StableSort(InViewModels, [Context, SortingPreset](const UItemInstanceViewModel* A, const UItemInstanceViewModel* B)
	{
		for (const TObjectPtr<UItemInstanceViewModelSorting>& SortAlgorithm : SortingPreset->SortingAlgorithms)
		{
			if (SortAlgorithm)
			{
				if (SortAlgorithm->GetResult(Context, A, B))
				{
					return true;
				}
			}
		}
		return false;
	});
}
