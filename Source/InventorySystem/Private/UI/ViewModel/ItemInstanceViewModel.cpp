// Copyright 2025 Soccertitan


#include "UI/ViewModel/ItemInstanceViewModel.h"

#include "Engine/AssetManager.h"
#include "Item/ItemDefinition.h"
#include "Item/Fragment/ItemFragment_UI.h"
#include "ItemContainer/ItemContainer.h"
#include "UI/ViewModel/ItemViewModel.h"

UItemInstanceViewModel::UItemInstanceViewModel()
{
	Bundles.Add("UI");
	ItemViewModelClass = UItemViewModel::StaticClass();
}

void UItemInstanceViewModel::SetItemInstance(const FItemInstance& ItemInstance)
{
	if (ItemInstance.IsValid())
	{
		bool bShouldLoadItemDefinition = false;
		if (Guid != ItemInstance.GetGuid())
		{
			ItemDefinitionStreamableHandle.Reset();
			bShouldLoadItemDefinition = true;
		}

		Guid = ItemInstance.GetGuid();
		Item = ItemInstance.GetItem();
		SetQuantity(ItemInstance.GetQuantity());
		ItemContainer = ItemInstance.GetItemContainer();
		InventoryManagerComponent = ItemContainer->GetInventoryManagerComponent();
		
		OnItemInstanceSet(ItemInstance);
		K2_OnItemInstanceSet(ItemInstance);

		if (bShouldLoadItemDefinition && bAutoLoadItemDefinition)
		{
			LoadItemDefinition();
		}
		else if (ItemViewModel)
		{
			ItemViewModel->SetItem(Item);
		}
	}
	else
	{
		ItemDefinitionStreamableHandle.Reset();
	}
}

void UItemInstanceViewModel::LoadItemDefinition()
{
	if (const FItem* ItemPtr = Item.GetPtr<FItem>())
	{
		FPrimaryAssetId AssetId = UAssetManager::Get().GetPrimaryAssetIdForPath(
		   ItemPtr->GetItemDefinition().ToSoftObjectPath());
	
		if (AssetId.IsValid())
		{
			FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this,
				&UItemInstanceViewModel::Internal_OnItemDefinitionLoaded, ItemPtr->GetItemDefinition());
			ItemDefinitionStreamableHandle = UAssetManager::Get().PreloadPrimaryAssets(
			   {AssetId}, Bundles, bLoadRecursive, Delegate);
		}
	}
}

void UItemInstanceViewModel::SetQuantity(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Quantity, InValue);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsAtMaxQuantity);
}

void UItemInstanceViewModel::SetMaxQuantity(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxQuantity, InValue);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsAtMaxQuantity);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CanHaveMaxQuantityGreaterThanOne);
}

void UItemInstanceViewModel::CreateItemViewModel(const UItemDefinition* ItemDefinition)
{
	const FItemFragment_UI* UIFrag = ItemDefinition->FindFragmentByType<FItemFragment_UI>();

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
	NewVM->SetItem(Item);
	UE_MVVM_SET_PROPERTY_VALUE(ItemViewModel, NewVM);
}

void UItemInstanceViewModel::Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UItemDefinition> ItemDefinition)
{
	if (UItemDefinition* Definition = ItemDefinition.Get())
	{
		CreateItemViewModel(Definition);
		SetMaxQuantity(GetItemContainer()->GetItemQuantityLimit(GetItem()));
		OnItemDefinitionLoaded(Definition);
		K2_OnItemDefinitionLoaded(Definition);
		
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(OnViewModelInitialized);
	}
	ItemDefinitionStreamableHandle.Reset();
}
