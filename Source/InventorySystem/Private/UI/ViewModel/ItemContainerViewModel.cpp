// Copyright Soccertitan 2025


#include "UI/ViewModel/ItemContainerViewModel.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "ItemContainer/ItemContainer.h"
#include "Item/ItemDefinition.h"
#include "InventoryManagerComponent.h"
#include "InventorySystem.h"
#include "Engine/AssetManager.h"
#include "UI/InventoryViewModelBlueprintFunctionLibrary.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"

UItemContainerViewModel::UItemContainerViewModel()
{
	Bundles.Add("ViewModel");
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
		if (IsValid(GetItemContainer()))
		{
			GetItemContainer()->OnItemAddedDelegate.RemoveAll(this);
			GetItemContainer()->OnItemRemovedDelegate.RemoveAll(this);
			GetItemContainer()->OnItemChangedDelegate.RemoveAll(this);
		}
		
		ItemContainer = InItemContainer;
		
		InItemContainer->OnItemAddedDelegate.AddUObject(this, &UItemContainerViewModel::Internal_OnItemAdded);
		InItemContainer->OnItemRemovedDelegate.AddUObject(this, &UItemContainerViewModel::Internal_OnItemRemoved);
		InItemContainer->OnItemChangedDelegate.AddUObject(this, &UItemContainerViewModel::Internal_OnItemChanged);
		
		const TArray<FItemInstance>& Items = GetItemContainer()->K2_GetItems();
		ItemInstanceViewModels.Empty(Items.Num());
		if (Items.Num() > 0)
		{
			SetIsLoadingInitialItems(true);
			LoadItemDefinitions(GetItemContainer()->K2_GetItems());
		}
		else
		{
			SetIsLoadingInitialItems(false);
		}
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(K2_GetItemInstanceViewModels);
		UE_MVVM_SET_PROPERTY_VALUE(ItemContainerName, GetItemContainer()->GetDisplayName());
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetMaxCapacity);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);
		
		OnItemContainerSet();
	}
}

void UItemContainerViewModel::SetIsLoadingInitialItems(bool bValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bLoadingInitialItems, bValue);
}

UItemContainer* UItemContainerViewModel::GetItemContainer() const
{
	return ItemContainer;
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

TArray<UItemInstanceViewModel*> UItemContainerViewModel::K2_GetItemInstanceViewModels() const
{
	return ItemInstanceViewModels;
}

const TArray<UItemInstanceViewModel*>& UItemContainerViewModel::GetItemInstanceViewModels() const
{
	return ItemInstanceViewModels;
}

void UItemContainerViewModel::LoadItemDefinitions(const TArray<FItemInstance>& ItemInstances)
{
	TArray<FPrimaryAssetId> AssetList;
	for (const FItemInstance& ItemInstance : ItemInstances)
	{
		FPrimaryAssetId AssetId = ItemInstance.GetItem().Get<FItem>().GetItemDefinition()->GetPrimaryAssetId();
		if (AssetId.IsValid())
		{
			AssetList.AddUnique(AssetId);
		}
	}
	
	if (AssetList.Num() > 0)
	{
		FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &UItemContainerViewModel::ItemDefinitionsLoaded, ItemInstances);
		UAssetManager::Get().PreloadPrimaryAssets(AssetList, Bundles, bLoadRecursive, Delegate);
	}
}

void UItemContainerViewModel::ItemDefinitionsLoaded(TArray<FItemInstance> ItemInstances)
{
	for (const FItemInstance& ItemInstance : ItemInstances)
	{
		UItemInstanceViewModel* NewViewModel = UInventoryViewModelBlueprintFunctionLibrary::CreateItemInstanceViewModel(this, ItemInstance);
		ItemInstanceViewModels.Add(NewViewModel);
		OnItemAdded(NewViewModel);

		if (!IsLoadingInitialItems())
		{
			ItemInstanceViewModelBuffer = NewViewModel;
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAddedItemInstanceViewModel);
			ItemInstanceViewModelBuffer = nullptr;
		}
	}
	
	if (IsLoadingInitialItems() && ItemInstanceViewModels.Num() >= GetConsumedCapacity())
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(K2_GetItemInstanceViewModels);
		SetIsLoadingInitialItems(false);
	}
}

void UItemContainerViewModel::Internal_OnItemAdded(const FItemInstance& ItemInstance)
{
	LoadItemDefinitions({ItemInstance});
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);
}

void UItemContainerViewModel::Internal_OnItemRemoved(const FItemInstance& ItemInstance)
{
	for (int32 idx = ItemInstanceViewModels.Num() - 1; idx >= 0; idx--)
	{
		if (ItemInstanceViewModels[idx]->GetHandle().GetGuid() == ItemInstance.GetGuid())
		{
			UItemInstanceViewModel* RemovedViewModel = ItemInstanceViewModels[idx];
			ItemInstanceViewModels.RemoveAtSwap(idx);
			OnItemRemoved(RemovedViewModel);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);

			ItemInstanceViewModelBuffer = RemovedViewModel;
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetRemovedItemInstanceViewModel);
			ItemInstanceViewModelBuffer = nullptr;
			break;
		}
	}
}

void UItemContainerViewModel::Internal_OnItemChanged(const FItemInstance& ItemInstance)
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
