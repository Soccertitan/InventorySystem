// Copyright Soccertitan 2025


#include "UI/ViewModel/ItemContainerViewModel.h"

#include "ItemContainer/ItemContainer.h"
#include "Item/ItemDefinition.h"
#include "InventoryManagerComponent.h"
#include "InventorySystem.h"
#include "Engine/AssetManager.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"

UItemContainerViewModel::UItemContainerViewModel()
{
	ItemInstanceViewModelClass = UItemInstanceViewModel::StaticClass();
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
		
		WeakItemContainer = InItemContainer;
		
		InItemContainer->OnItemAddedDelegate.AddUObject(this, &UItemContainerViewModel::Internal_OnItemAdded);
		InItemContainer->OnItemRemovedDelegate.AddUObject(this, &UItemContainerViewModel::Internal_OnItemRemoved);
		InItemContainer->OnItemChangedDelegate.AddUObject(this, &UItemContainerViewModel::Internal_OnItemChanged);

		ItemInstanceViewModels.Empty(GetItemContainer()->GetItems().Num());
		for (const FItemInstance& ItemInstance : GetItemContainer()->GetItems())
		{
			if (DoesItemHaveUIFragment(ItemInstance.GetItem()))
			{
				UItemInstanceViewModel* NewVM = CreateItemInstanceViewModel(ItemInstance);
				ItemInstanceViewModels.Add(NewVM);
			}
		}
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemInstanceViewModels);
		UE_MVVM_SET_PROPERTY_VALUE(ItemContainerName, GetItemContainer()->GetDisplayName());
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetMaxCapacity);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);
		
		OnItemContainerSet();
	}
}

UItemContainer* UItemContainerViewModel::GetItemContainer() const
{
	return WeakItemContainer.Get();
}

int32 UItemContainerViewModel::GetConsumedCapacity() const
{
	return GetItemContainer()->GetConsumedCapacity();
}

int32 UItemContainerViewModel::GetMaxCapacity() const
{
	return GetItemContainer()->GetMaxCapacity();
}

TArray<UItemInstanceViewModel*> UItemContainerViewModel::GetItemInstanceViewModels() const
{
	return ItemInstanceViewModels;
}

void UItemContainerViewModel::Internal_OnItemAdded(UItemContainer* InContainer, const FItemInstance& ItemInstance)
{
	if (DoesItemHaveUIFragment(ItemInstance.GetItem()))
	{
		UItemInstanceViewModel* NewViewModel = CreateItemInstanceViewModel(ItemInstance);
		ItemInstanceViewModels.Add(NewViewModel);
		OnItemAdded(ItemInstance, NewViewModel);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);

		ItemInstanceViewModelBuffer = NewViewModel;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAddedItemInstanceViewModel);
		ItemInstanceViewModelBuffer = nullptr;
	}
}

void UItemContainerViewModel::Internal_OnItemRemoved(UItemContainer* InContainer, const FItemInstance& ItemInstance)
{
	for (int32 idx = ItemInstanceViewModels.Num() - 1; idx >= 0; idx--)
	{
		if (ItemInstanceViewModels[idx]->GetGuid() == ItemInstance.GetGuid())
		{
			UItemInstanceViewModel* RemovedViewModel = ItemInstanceViewModels[idx];
			ItemInstanceViewModels.RemoveAt(idx);
			OnItemRemoved(ItemInstance, RemovedViewModel);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);

			ItemInstanceViewModelBuffer = RemovedViewModel;
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetRemovedItemInstanceViewModel);
			ItemInstanceViewModelBuffer = nullptr;
			break;
		}
	}
}

void UItemContainerViewModel::Internal_OnItemChanged(UItemContainer* InContainer, const FItemInstance& ItemInstance)
{
	for (UItemInstanceViewModel* ItemInstanceViewModel : ItemInstanceViewModels)
	{
		if (ItemInstanceViewModel->GetGuid() == ItemInstance.GetGuid())
		{
			ItemInstanceViewModel->SetItemInstance(ItemInstance);
			OnItemChanged(ItemInstance, ItemInstanceViewModel);

			ItemInstanceViewModelBuffer = ItemInstanceViewModel;
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetChangedItemInstanceViewModel);
			ItemInstanceViewModelBuffer = nullptr;
			break;
		}
	}
}

UItemInstanceViewModel* UItemContainerViewModel::CreateItemInstanceViewModel(const FItemInstance& ItemInstance)
{
	if (ItemInstanceViewModelClass)
	{
		UItemInstanceViewModel* NewVM = NewObject<UItemInstanceViewModel>(this, ItemInstanceViewModelClass);
		NewVM->SetItemInstance(ItemInstance);
		return NewVM;
	}
	return nullptr;
}

bool UItemContainerViewModel::DoesItemHaveUIFragment(const TInstancedStruct<FItem>& Item)
{
	FAssetData AssetData;
	bool Result;
	FPrimaryAssetId AssetId = Item.Get<FItem>().GetItemDefinition()->GetPrimaryAssetId();
	UAssetManager::Get().GetPrimaryAssetData(AssetId, AssetData);
	AssetData.GetTagValue("ItemFragment_UI", Result);
	return Result;
}
