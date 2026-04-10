// Copyright 2025 Soccertitan


#include "UI/ViewModel/ItemInstanceViewModel.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "InventoryManagerComponent.h"
#include "Engine/AssetManager.h"
#include "Item/ItemDefinition.h"
#include "Item/Fragment/ItemFragment_UI.h"
#include "UI/ViewModel/ItemViewModel.h"

UItemInstanceViewModel::UItemInstanceViewModel()
{
}

void UItemInstanceViewModel::SetItemInstance(const FItemInstance& InItemInstance)
{
	if (InItemInstance.IsValid())
	{
		bool bShouldLoadItemDefinition = false;
		if (ItemInstance != InItemInstance)
		{
			ItemDefinitionStreamableHandle.Reset();
			bShouldLoadItemDefinition = true;
		}

		ItemInstance = InItemInstance;
		SetQuantity(ItemInstance.GetQuantity());

		if (bShouldLoadItemDefinition)
		{
			LoadItemDefinition(ItemInstance.GetItem().Get<FItem>().GetItemDefinition());
		}
		else
		{
			if (ItemViewModel)
			{
				ItemViewModel->SetItem(ItemInstance.GetItem());
			}
		}
	}
	else
	{
		ItemViewModel = NewObject<UItemViewModel>(this);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ItemViewModel);
		ItemInstance = FItemInstance();
		SetQuantity(0);
	}
	
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsItemInstanceValid);
}

void UItemInstanceViewModel::SetQuantity(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Quantity, InValue);
}

void UItemInstanceViewModel::LoadItemDefinition(TSoftObjectPtr<UItemDefinition> ItemDefinition)
{
	if (!ItemDefinition.IsNull())
	{
		FPrimaryAssetId AssetId = UAssetManager::Get().GetPrimaryAssetIdForPath(
			ItemDefinition.ToSoftObjectPath());
		
		if (AssetId.IsValid())
		{
			FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &UItemInstanceViewModel::Internal_OnItemDefinitionLoaded, ItemDefinition);
			ItemDefinitionStreamableHandle = UAssetManager::Get().PreloadPrimaryAssets(
			   {AssetId}, {"ViewModel"}, false, Delegate);
		}
	}
}

void UItemInstanceViewModel::Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UItemDefinition> ItemDefinitionSoft)
{
	if (const UItemDefinition* ItemDefinition = ItemDefinitionSoft.Get())
	{
		if (const FItemFragment_UI* UIFrag = ItemDefinition->FindFragmentByType<FItemFragment_UI>())
		{
			TSubclassOf<UItemViewModel> ItemViewModelClass = UIFrag->ItemViewModelClass.Get();
			if (!ItemViewModelClass)
			{
				ItemViewModelClass = UItemViewModel::StaticClass();
			}
			
			ItemViewModel = NewObject<UItemViewModel>(this, ItemViewModelClass);
			ItemViewModel->SetItemInstance(this);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ItemViewModel);
		}
	}
	ItemDefinitionStreamableHandle.Reset();
}
