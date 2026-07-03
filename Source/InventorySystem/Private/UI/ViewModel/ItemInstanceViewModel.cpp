// Copyright 2025 Soccertitan


#include "UI/ViewModel/ItemInstanceViewModel.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "InventoryManagerComponent.h"
#include "InventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "Item/ItemDefinition.h"
#include "ItemContainer/ItemContainer.h"
#include "UI/ItemViewModelInterface.h"
#include "UI/ViewModel/Component/ItemInstanceComponentViewModel.h"

UItemInstanceViewModel::UItemInstanceViewModel()
{
	Bundles.Append({"UI", "ViewModel"});
}

void UItemInstanceViewModel::SetItemInstance(const FItemInstance& ItemInstance)
{
	if (ItemInstance.IsValid())
	{
		bool bShouldLoadItemDefinition = false;
		if (ItemInstance.GetHandle() != Handle)
		{
			if (ItemInstance.GetHandle().GetGuid() != Handle.GetGuid())
			{
				ItemDefinitionStreamableHandle.Reset();
				bShouldLoadItemDefinition = true;
			}
			Handle = ItemInstance.GetHandle();
		}
		
		OnItemInstanceSet(ItemInstance);
		for (UItemInstanceComponentViewModel* ViewModel : ItemInstanceComponentViewModels)
		{
			ViewModel->OnItemInstanceSet(ItemInstance);
		}

		SetItem(ItemInstance.GetItem(), bShouldLoadItemDefinition);
	}
}

void UItemInstanceViewModel::SetItem(const TInstancedStruct<FItem>& Item, bool bShouldSetItemDefinition)
{
	if (Item.IsValid())
	{
		CachedItem = Item;
		OnItemSet(CachedItem);
		for (UItemInstanceComponentViewModel* ViewModel : ItemInstanceComponentViewModels)
		{
			ViewModel->OnItemSet(Item);
		}
		
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
		const bool NewItemDef = ItemDefinitionSoft.Get() != ItemDefinition;
		CreateItemInstanceComponentViewModelsInternal(ItemDefinition, NewItemDef);
		
		ItemDefinitionSoft = ItemDefinition->GetPathName();
		OnItemDefinitionSet(ItemDefinition);
		for (UItemInstanceComponentViewModel* ViewModel : ItemInstanceComponentViewModels)
		{
			ViewModel->OnItemDefinitionSet(ItemDefinition);
		}
	}
}

UItemInstanceComponentViewModel* UItemInstanceViewModel::K2_FindOrCreateItemInstanceComponentViewModel(TSubclassOf<UItemInstanceComponentViewModel> Class)
{
	if (Class)
	{
		for (UItemInstanceComponentViewModel* ViewModel : ItemInstanceComponentViewModels)
		{
			if (ViewModel->IsA(Class))
			{
				return ViewModel;
			}
		}
		UItemInstanceComponentViewModel* NewVM = NewObject<UItemInstanceComponentViewModel>(this, Class);
		return NewVM;
	}
	return nullptr;
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
			WidgetClass = ItemDefinition->WidgetClass.Get();
			if (!WidgetClass && !ItemDefinition->WidgetClass.IsNull())
			{
				UAssetManager::Get().LoadAssetList({ItemDefinition->WidgetClass.ToSoftObjectPath()})->WaitUntilComplete();
				WidgetClass = ItemDefinition->WidgetClass.Get();
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
			FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &UItemInstanceViewModel::OnItemDefinitionLoadedInternal);
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
	SetDescription(ItemDefinition->Description);
	SetIcon(ItemDefinition->Icon);
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

void UItemInstanceViewModel::OnItemDefinitionLoadedInternal()
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

void UItemInstanceViewModel::CreateItemInstanceComponentViewModelsInternal(const UItemDefinition* ItemDefinition, bool bReset)
{
	if (bReset)
	{
		ItemInstanceComponentViewModels.Reset();
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemInstanceComponentViewModels);
	}
	
	bool bAddedNewViewModels = false;
	for (TInstancedStruct<FItemDefinitionFragment> Fragment : ItemDefinition->Fragments)
	{
		if (const FItemDefinitionFragment* FragmentPtr = Fragment.GetPtr<FItemDefinitionFragment>())
		{
			TSubclassOf<UItemInstanceComponentViewModel> ComponentViewModelClass = Fragment->GetItemInstanceComponentViewModel();
			UItemInstanceComponentViewModel* ComponentViewModel = K2_FindOrCreateItemInstanceComponentViewModel(ComponentViewModelClass);
			if (!ComponentViewModel)
			{
				continue;
			}
			ItemInstanceComponentViewModels.Add(ComponentViewModel);
			bAddedNewViewModels = true;
			if (FItemInstance* ItemInstance = Handle.GetItemInstance())
			{
				ComponentViewModel->OnItemInstanceSet(*ItemInstance);
			}
			if (CachedItem.IsValid())
			{
				ComponentViewModel->OnItemSet(CachedItem);
			}
			ComponentViewModel->OnItemDefinitionSet(ItemDefinition);
		}
	}
	if (bAddedNewViewModels)
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemInstanceComponentViewModels);
	}
}
