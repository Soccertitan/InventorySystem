// Copyright 2025 Soccertitan


#include "UI/ViewModel/ItemInstanceViewModel.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "InventoryManagerComponent.h"
#include "InventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "Item/ItemDefinition.h"
#include "Item/Fragment/ItemDefinitionFragment_UI.h"
#include "ItemContainer/ItemContainer.h"
#include "UI/ItemViewModelInterface.h"

UItemInstanceViewModel::UItemInstanceViewModel()
{
	Bundles.Add("UI");
}

void UItemInstanceViewModel::SetItemInstance(const FItemInstance& ItemInstance)
{
	if (ItemInstance.IsValid())
	{
		bool bShouldLoadItemDefinition = false;
		if (ItemInstance.GetHandle() != Handle)
		{
			ItemDefinitionStreamableHandle.Reset();
			bShouldLoadItemDefinition = true;
			Handle = ItemInstance.GetHandle();
		}
		
		OnItemInstanceSet(ItemInstance);

		SetItem(ItemInstance.GetItem(), bShouldLoadItemDefinition);
	}
}

void UItemInstanceViewModel::SetItem(const TInstancedStruct<FItem>& Item, bool bShouldSetItemDefinition)
{
	if (Item.IsValid())
	{
		CachedItem = Item;
		OnItemSet(CachedItem);
		
		if (bShouldSetItemDefinition)
		{
			ItemDefinitionSoft = CachedItem.Get<FItem>().GetItemDefinition();
			SetItemDefinition(UInventoryBlueprintFunctionLibrary::GetItemDefinition(CachedItem));
			
			if (bAutoLoadItemDefinition)
			{
				LoadItemDefinition();
			}
		}
	}
}

void UItemInstanceViewModel::SetItemDefinition(const UItemDefinition* ItemDefinition)
{
	if (ItemDefinition)
	{
		ItemDefinitionSoft = ItemDefinition->GetPathName();
		OnItemDefinitionSet(ItemDefinition);
	}
}

UUserWidget* UItemInstanceViewModel::CreateItemDetailsWidget(APlayerController* OwningPlayer, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass && !ItemDefinitionSoft.IsNull())
	{
		const UItemDefinition* ItemDefinition = ItemDefinitionSoft.Get();
		if (!ItemDefinition)
		{
			UAssetManager::Get().LoadAssetList({ItemDefinitionSoft.ToSoftObjectPath()})->WaitUntilComplete();
		}
		if (ItemDefinition)
		{
			if (const FItemDefinitionFragment_UI* Fragment = ItemDefinition->FindFragmentByType<FItemDefinitionFragment_UI>())
			{
				WidgetClass = Fragment->WidgetClass.Get();
				if (!WidgetClass && !Fragment->WidgetClass.IsNull())
				{
					UAssetManager::Get().LoadAssetList({Fragment->WidgetClass.ToSoftObjectPath()})->WaitUntilComplete();
					WidgetClass = Fragment->WidgetClass.Get();
				}
			}
		}
	}

	if (WidgetClass)
	{
		UUserWidget* NewWidget = CreateWidget<UUserWidget>(OwningPlayer, WidgetClass);
		if (NewWidget->Implements<UItemViewModelInterface>())
		{
			IItemViewModelInterface::Execute_SetItemViewModel(NewWidget, this);
		}
		else
		{
			UE_LOG(LogInventorySystem, Error, TEXT("[%s] does not implement ItemViewModelInterface."), *GetNameSafe(NewWidget));
		}
		return NewWidget;
	}
	return nullptr;
}

void UItemInstanceViewModel::LoadItemDefinition()
{
	ItemDefinitionStreamableHandle.Reset();
	if (!ItemDefinitionSoft.IsNull())
	{
		FPrimaryAssetId AssetId = UAssetManager::Get().GetPrimaryAssetIdForPath(ItemDefinitionSoft.ToSoftObjectPath());
	
		if (AssetId.IsValid())
		{
			FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &UItemInstanceViewModel::Internal_OnItemDefinitionLoaded);
			ItemDefinitionStreamableHandle = UAssetManager::Get().PreloadPrimaryAssets(
			   {AssetId}, Bundles, bLoadRecursive, Delegate);
		}
	}
}

void UItemInstanceViewModel::ReleaseItemDefinitionHandle()
{
	ItemDefinitionStreamableHandle.Reset();
}

void UItemInstanceViewModel::OnItemInstanceSet_Implementation(const FItemInstance& ItemInstance)
{
	SetQuantity(ItemInstance.GetQuantity());
}

void UItemInstanceViewModel::OnItemSet_Implementation(const TInstancedStruct<FItem>& Item)
{
	if (GetItemContainer())
	{
		SetMaxQuantity(GetItemContainer()->GetItemQuantityLimit(Item));
	}
	else
	{
		SetMaxQuantity(0);
	}
}

void UItemInstanceViewModel::OnItemDefinitionSet_Implementation(const UItemDefinition* ItemDefinition)
{
	SetItemName(ItemDefinition->ItemName);
	if (const FItemDefinitionFragment_UI* UIFrag = ItemDefinition->FindFragmentByType<FItemDefinitionFragment_UI>())
	{
		SetDescription(UIFrag->Description);
		SetIcon(UIFrag->Icon);
	}
}

void UItemInstanceViewModel::SetItemName(FText InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ItemName, InValue);
}

void UItemInstanceViewModel::SetDescription(FText InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Description, InValue);
}

void UItemInstanceViewModel::SetIcon(const TSoftObjectPtr<UTexture2D>& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Icon, InValue);
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

void UItemInstanceViewModel::Internal_OnItemDefinitionLoaded()
{
	if (const UItemDefinition* ItemDefinition = ItemDefinitionSoft.Get())
	{
		SetItemDefinition(ItemDefinition);
	}
	
	if (bAutoUnloadItemDefinition)
	{
		ItemDefinitionStreamableHandle.Reset();
	}
}
