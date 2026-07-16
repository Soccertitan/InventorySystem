// Copyright Soccertitan 2025


#include "UI/ViewModel/ItemContainerViewModel.h"

#include "ItemContainer/ItemContainer.h"
#include "InventoryManagerComponent.h"
#include "InventorySettings.h"
#include "InventorySystem.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"
#include "UI/ViewModel/Sorting/ItemInstanceViewModelSorting.h"
#include "UI/ViewModel/Sorting/ItemInstanceViewModelSortingPreset.h"


UItemContainerViewModel::UItemContainerViewModel()
{
}

void UItemContainerViewModel::SetItemContainer(UItemContainer* InItemContainer)
{
	if (!InItemContainer)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("The InItemContainer is null. [%s]"), *GetName());
		return;
	}

	if (InItemContainer != GetItemContainer())
	{
		if (ItemContainer)
		{
			ItemContainer->OnItemAddedDelegate.RemoveAll(this);
			ItemContainer->OnItemRemovedDelegate.RemoveAll(this);
			ItemContainer->OnItemChangedDelegate.RemoveAll(this);
		}
		
		ItemContainer = InItemContainer;
		
		InItemContainer->OnItemAddedDelegate.AddUObject(this, &UItemContainerViewModel::OnItemAddedInternal);
		InItemContainer->OnItemRemovedDelegate.AddUObject(this, &UItemContainerViewModel::OnItemRemovedInternal);
		InItemContainer->OnItemChangedDelegate.AddUObject(this, &UItemContainerViewModel::OnItemChangedInternal);
		
		const TArray<FItemInstance>& Items = GetItemContainer()->K2_GetItems();
		ItemInstanceViewModels.Empty(Items.Num());
		for (const FItemInstance& ItemInstance : Items)
		{
			UItemInstanceViewModel* NewViewModel = NewObject<UItemInstanceViewModel>(this, GetItemInstanceViewModelClass(ItemInstance));
			NewViewModel->SetItemInstance(ItemInstance);
			ItemInstanceViewModels.Add(NewViewModel);
			OnItemAdded(NewViewModel);
		}
		
		bool bAutoSortCompleted = false;
		if (bAutoSort)
		{
			bAutoSortCompleted = SortItemInstances(GetAutoSortContextObject(), AutoSortPreset);
		}
		if (!bAutoSortCompleted)
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(K2_GetItemInstanceViewModels);
		}
		
		UE_MVVM_SET_PROPERTY_VALUE(ItemContainerName, GetItemContainer()->GetDisplayName());
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetMaxCapacity);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);
		
		OnItemContainerSet();
	}
}

UItemContainer* UItemContainerViewModel::GetItemContainer() const
{
	return ItemContainer;
}

void UItemContainerViewModel::SetAutoSortEnabled(bool bEnabled)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bAutoSort, bEnabled))
	{
		if (bAutoSort)
		{
			SortItemInstances(GetAutoSortContextObject(), AutoSortPreset);
		}
	}
}

void UItemContainerViewModel::SetAutoSortPreset(UItemInstanceViewModelSortingPreset* Value)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(AutoSortPreset, Value))
	{
		if (bAutoSort)
		{
			SortItemInstances(GetAutoSortContextObject(), AutoSortPreset);
		}
	}
}

bool UItemContainerViewModel::SortItemInstances(const UObject* Context, const UItemInstanceViewModelSortingPreset* SortingPreset)
{
	if (SortingPreset && ItemInstanceViewModels.Num() > 0)
	{
		Algo::StableSort(ItemInstanceViewModels, [Context, SortingPreset](const UItemInstanceViewModel* A, const UItemInstanceViewModel* B)
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
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(K2_GetItemInstanceViewModels);
		return true;
	}
	return false;
}

int32 UItemContainerViewModel::GetConsumedCapacity() const
{
	if (GetItemContainer())
	{
		return GetItemContainer()->GetConsumedCapacity();
	}
	
	return 0;
}

int32 UItemContainerViewModel::GetMaxCapacity() const
{
	if (GetItemContainer())
	{
		return GetItemContainer()->GetMaxCapacity();
	}
	return 0;
}

TSubclassOf<UItemInstanceViewModel> UItemContainerViewModel::GetItemInstanceViewModelClass(const FItemInstance& ItemInstance) const
{
	return UInventorySettings::GetItemInstanceViewModelClass();
}

TArray<UItemInstanceViewModel*> UItemContainerViewModel::K2_GetItemInstanceViewModels() const
{
	return ItemInstanceViewModels;
}

const TArray<UItemInstanceViewModel*>& UItemContainerViewModel::GetItemInstanceViewModels() const
{
	return ItemInstanceViewModels;
}

void UItemContainerViewModel::OnItemAddedInternal(const FItemInstance& ItemInstance)
{
	UItemInstanceViewModel* NewViewModel = NewObject<UItemInstanceViewModel>(this, GetItemInstanceViewModelClass(ItemInstance));
	NewViewModel->SetItemInstance(ItemInstance);
	ItemInstanceViewModels.Add(NewViewModel);
	OnItemAdded(NewViewModel);

	bool bAutoSortCompleted = false;
	if (bAutoSort)
	{
		bAutoSortCompleted = SortItemInstances(GetAutoSortContextObject(), AutoSortPreset);
	}
	
	if (!bAutoSortCompleted)
	{
		ItemInstanceViewModelBuffer = NewViewModel;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAddedItemInstanceViewModel);
		ItemInstanceViewModelBuffer = nullptr;
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);
}

void UItemContainerViewModel::OnItemRemovedInternal(const FItemInstance& ItemInstance)
{
	for (int32 idx = ItemInstanceViewModels.Num() - 1; idx >= 0; idx--)
	{
		if (ItemInstanceViewModels[idx]->GetHandle().GetGuid() == ItemInstance.GetGuid())
		{
			UItemInstanceViewModel* RemovedViewModel = ItemInstanceViewModels[idx];
			ItemInstanceViewModels.RemoveAt(idx);
			OnItemRemoved(RemovedViewModel);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);

			ItemInstanceViewModelBuffer = RemovedViewModel;
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetRemovedItemInstanceViewModel);
			ItemInstanceViewModelBuffer = nullptr;
			break;
		}
	}
}

void UItemContainerViewModel::OnItemChangedInternal(const FItemInstance& ItemInstance)
{
	for (UItemInstanceViewModel* ItemInstanceViewModel : ItemInstanceViewModels)
	{
		if (ItemInstanceViewModel->GetHandle().GetGuid() == ItemInstance.GetGuid())
		{
			ItemInstanceViewModel->SetItemInstance(ItemInstance);
			OnItemChanged(ItemInstanceViewModel);
			break;
		}
	}
}
