// Copyright Soccertitan 2025


#include "UI/ViewModel/ItemContainerViewModel.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "ItemContainer/ItemContainer.h"
#include "Item/ItemDefinition.h"
#include "InventoryManagerComponent.h"
#include "InventorySystem.h"
#include "Engine/AssetManager.h"
#include "Item/Fragment/ItemFragment_UI.h"
#include "UI/InventoryViewModelBlueprintFunctionLibrary.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"

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
		if (IsValid(GetItemContainer()))
		{
			GetItemContainer()->OnItemAddedDelegate.RemoveAll(this);
			GetItemContainer()->OnItemRemovedDelegate.RemoveAll(this);
		}
		
		ItemContainer = InItemContainer;
		
		InItemContainer->OnItemAddedDelegate.AddUObject(this, &UItemContainerViewModel::Internal_OnItemAdded);
		InItemContainer->OnItemRemovedDelegate.AddUObject(this, &UItemContainerViewModel::Internal_OnItemRemoved);

		ItemInstanceViewModels.Empty(GetItemContainer()->GetItems().Num());
		LoadItemDefinitions(GetItemContainer()->GetItems());
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(K2_GetItemInstanceViewModels);
		UE_MVVM_SET_PROPERTY_VALUE(ItemContainerName, GetItemContainer()->GetDisplayName());
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetMaxCapacity);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);
		
		OnItemContainerSet();
	}
}

UItemContainer* UItemContainerViewModel::GetItemContainer() const
{
	return ItemContainer.Get();
}

int32 UItemContainerViewModel::GetConsumedCapacity() const
{
	return GetItemContainer()->GetConsumedCapacity();
}

int32 UItemContainerViewModel::GetMaxCapacity() const
{
	return GetItemContainer()->GetMaxCapacity();
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
	
	FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &UItemContainerViewModel::ItemDefinitionsLoaded, ItemInstances);
	UAssetManager::Get().PreloadPrimaryAssets(AssetList, {"ViewModel"}, false, Delegate);
}

void UItemContainerViewModel::ItemDefinitionsLoaded(TArray<FItemInstance> ItemInstances)
{
	for (const FItemInstance& ItemInstance : ItemInstances)
	{
		UItemInstanceViewModel* NewViewModel = UInventoryViewModelBlueprintFunctionLibrary::CreateItemInstanceViewModel(this, ItemInstance);
		ItemInstanceViewModels.Add(NewViewModel);
		OnItemAdded(NewViewModel);

		ItemInstanceViewModelBuffer = NewViewModel;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAddedItemInstanceViewModel);
		ItemInstanceViewModelBuffer = nullptr;
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
		if (ItemInstanceViewModels[idx]->GetItemInstance() == ItemInstance)
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

bool UItemContainerViewModel::DoesItemHaveUIFragment(const TInstancedStruct<FItem>& Item)
{
	FAssetData AssetData;
	bool Result;
	FPrimaryAssetId AssetId = Item.Get<FItem>().GetItemDefinition()->GetPrimaryAssetId();
	UAssetManager::Get().GetPrimaryAssetData(AssetId, AssetData);
	AssetData.GetTagValue("ItemFragment_UI", Result);
	return Result;
}
