// Copyright Soccertitan 2025


#include "UI/InventoryViewModelBlueprintFunctionLibrary.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "InventoryFastTypes.h"
#include "Engine/AssetManager.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"
#include "UI/ViewModel/Sorting/ItemInstanceViewModelSortingAlgorithm.h"
#include "UI/ViewModel/Sorting/ItemInstanceViewModelSortingPreset.h"

UItemInstanceViewModel* UInventoryViewModelBlueprintFunctionLibrary::CreateItemInstanceViewModel(UObject* Owner, const FItemInstance& ItemInstance)
{
	if (Owner)
	{
		TSubclassOf<UItemInstanceViewModel> ItemInstanceViewModelClass = UItemInstanceViewModel::StaticClass();
		if (ItemInstance.IsValid())
		{
			if (const UItemDefinition* ItemDefinition = UInventoryBlueprintFunctionLibrary::GetItemDefinition(ItemInstance.GetItem()))
			{
				if (!ItemDefinition->ItemInstanceViewModelClass.Get())
				{
					UAssetManager::Get().LoadAssetList({ItemDefinition->ItemInstanceViewModelClass.ToSoftObjectPath()})->WaitUntilComplete();
				}
				ItemInstanceViewModelClass = ItemDefinition->ItemInstanceViewModelClass.Get() ? ItemDefinition->ItemInstanceViewModelClass.Get() : UItemInstanceViewModel::StaticClass();
			}
		}
		UItemInstanceViewModel* NewVM = NewObject<UItemInstanceViewModel>(Owner, ItemInstanceViewModelClass);
		NewVM->SetItemInstance(ItemInstance);
		return NewVM;
	}
	return nullptr;
}

void UInventoryViewModelBlueprintFunctionLibrary::SortItemInstanceViewModels(const UItemInstanceViewModelSortingPreset* SortingPreset, TArray<UItemInstanceViewModel*>& InViewModels)
{
	if (!SortingPreset || InViewModels.IsEmpty())
	{
		return;
	}

	Algo::Sort(InViewModels, [SortingPreset](const UItemInstanceViewModel* A, const UItemInstanceViewModel* B)
	{
		for (const TObjectPtr<UItemInstanceViewModelSortingAlgorithm>& SortAlgorithm : SortingPreset->SortingAlgorithms)
		{
			if (SortAlgorithm)
			{
				if (SortAlgorithm->GetResult(A, B))
				{
					return true;
				}
			}
		}
		return false;
	});
}
