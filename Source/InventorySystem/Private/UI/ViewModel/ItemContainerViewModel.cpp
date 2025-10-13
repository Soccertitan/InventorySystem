// Copyright Soccertitan


#include "UI/ViewModel/ItemContainerViewModel.h"

#include "ItemContainer/ItemContainer.h"
#include "Item/ItemDefinition.h"
#include "InventoryManagerComponent.h"
#include "InventoryBlueprintFunctionLibrary.h"
#include "InventorySystem.h"
#include "Engine/AssetManager.h"
#include "Item/Fragment/ItemFragment_UI.h"
#include "UI/ViewModel/ItemViewModel.h"

UItemContainerViewModel::UItemContainerViewModel()
{
	ItemViewModelClass = UItemViewModel::StaticClass();
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

		ItemViewModels.Empty(GetItemContainer()->GetItems().Num());
		for (const FItemInstance& ItemInstance : GetItemContainer()->GetItems())
		{
			if (DoesItemHaveUIFragment(ItemInstance.Item))
			{
				UItemViewModel* NewVM = CreateItemViewModel(ItemInstance);
				ItemViewModels.Add(NewVM);
			}
		}
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemViewModels);
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

TArray<UItemViewModel*> UItemContainerViewModel::GetItemViewModels() const
{
	return ItemViewModels;
}

void UItemContainerViewModel::Internal_OnItemAdded(UItemContainer* InContainer, const FItemInstance& ItemInstance)
{
	if (DoesItemHaveUIFragment(ItemInstance.Item))
	{
		UItemViewModel* NewViewModel = CreateItemViewModel(ItemInstance);
		ItemViewModels.Add(NewViewModel);
		OnItemAdded(ItemInstance, NewViewModel);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);

		ItemViewModelBuffer = NewViewModel;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAddedItemViewModel);
		ItemViewModelBuffer = nullptr;
	}
}

void UItemContainerViewModel::Internal_OnItemRemoved(UItemContainer* InContainer, const FItemInstance& ItemInstance)
{
	for (int32 idx = ItemViewModels.Num() - 1; idx >= 0; idx--)
	{
		if (ItemViewModels[idx]->GetGuid() == ItemInstance.GetGuid())
		{
			UItemViewModel* RemovedViewModel = ItemViewModels[idx];
			ItemViewModels.RemoveAt(idx);
			OnItemRemoved(ItemInstance, RemovedViewModel);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);

			ItemViewModelBuffer = RemovedViewModel;
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetRemovedItemViewModel);
			ItemViewModelBuffer = nullptr;
			break;
		}
	}
}

void UItemContainerViewModel::Internal_OnItemChanged(UItemContainer* InContainer, const FItemInstance& ItemInstance)
{
	for (UItemViewModel* ItemViewModel : ItemViewModels)
	{
		if (ItemViewModel->GetGuid() == ItemInstance.GetGuid())
		{
			ItemViewModel->SetItemInstance(ItemInstance);
			OnItemChanged(ItemInstance, ItemViewModel);

			ItemViewModelBuffer = ItemViewModel;
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetChangedItemViewModel);
			ItemViewModelBuffer = nullptr;
			break;
		}
	}
}

UItemViewModel* UItemContainerViewModel::CreateItemViewModel(const FItemInstance& ItemInstance)
{
	const UItemDefinition* ItemDef = UInventoryBlueprintFunctionLibrary::GetItemDefinition(ItemInstance.Item);
	const FItemFragment_UI* UIFrag = ItemDef->FindFragmentByType<FItemFragment_UI>();

	TSubclassOf<UItemViewModel> ViewModelClass = ItemViewModelClass;
	if (UIFrag && !UIFrag->ItemViewModelClass.IsNull())
	{
		if (!UIFrag->ItemViewModelClass.Get())
		{
			UAssetManager::Get().LoadAssetList({
			UIFrag->ItemViewModelClass.ToSoftObjectPath()})->WaitUntilComplete();
		}
		ViewModelClass = UIFrag->ItemViewModelClass.Get();
	}

	UItemViewModel* NewVM = NewObject<UItemViewModel>(this, ViewModelClass);
	NewVM->SetItemInstance(ItemInstance);
	return NewVM;
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
