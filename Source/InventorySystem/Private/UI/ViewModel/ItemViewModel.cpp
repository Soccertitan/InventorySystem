// Copyright Soccertitan 2025


#include "UI/ViewModel/ItemViewModel.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "InventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "Item/Fragment/ItemFragment_UI.h"
#include "ItemContainer/ItemContainer.h"
#include "UI/ItemViewModelInterface.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"

UItemViewModel::UItemViewModel()
{
	Bundles.Add("UI");
}

void UItemViewModel::LoadItemDefinition(TSoftObjectPtr<UItemDefinition> ItemDefinition)
{
	if (!ItemDefinition.IsNull())
	{
		FPrimaryAssetId AssetId = UAssetManager::Get().GetPrimaryAssetIdForPath(
			ItemDefinition.ToSoftObjectPath());
		
		if (AssetId.IsValid())
		{
			FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &UItemViewModel::Internal_OnItemDefinitionLoaded, ItemDefinition);
			ItemDefinitionStreamableHandle = UAssetManager::Get().PreloadPrimaryAssets(
			   {AssetId}, Bundles, bLoadRecursive, Delegate);
		}
	}
}

void UItemViewModel::ReleaseItemDefinitionHandle()
{
	ItemDefinitionStreamableHandle.Reset();
}

UUserWidget* UItemViewModel::CreateItemDetailsWidget(APlayerController* OwningPlayer, TSubclassOf<UUserWidget> WidgetClass, bool bForceWidgetClass)
{
	TSubclassOf<UUserWidget> WidgetClassToCreate = WidgetClass;
	if (!WidgetClassToCreate || bForceWidgetClass == false)
	{
		if (!ItemDefinitionSoft.IsNull())
		{
			FPrimaryAssetId AssetId = UAssetManager::Get().GetPrimaryAssetIdForPath(ItemDefinitionSoft.ToSoftObjectPath());
			if (AssetId.IsValid())
			{
				UAssetManager::Get().PreloadPrimaryAssets({AssetId}, {"UI"}, false)->WaitUntilComplete();
				if (const FItemFragment_UI* Fragment = ItemDefinitionSoft.Get()->FindFragmentByType<FItemFragment_UI>())
				{
					WidgetClassToCreate = Fragment->WidgetClass.Get();
				}
			}
		}
	}

	if (WidgetClassToCreate)
	{
		UUserWidget* NewWidget = CreateWidget<UUserWidget>(OwningPlayer, WidgetClassToCreate);
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

void UItemViewModel::SetItemInstance(UItemInstanceViewModel* ViewModel)
{
	if (ViewModel)
	{
		if (ViewModel != ItemInstanceViewModel)
		{
			ItemDefinitionStreamableHandle.Reset();
			ItemInstanceViewModel = ViewModel;
		}
		
		OnItemInstanceSet();
		K2_OnItemInstanceSet();
		
		SetItem(ItemInstanceViewModel->GetItemInstance().GetItem());
	}
}

void UItemViewModel::SetItem(const TInstancedStruct<FItem>& Item)
{
	if (Item.IsValid())
	{
		OnItemSet(Item);
		K2_OnItemSet(Item);
	
		if (Item.Get<FItem>().GetItemDefinition() != ItemDefinitionSoft)
		{
			ItemDefinitionSoft = Item.Get<FItem>().GetItemDefinition();
			if (!ItemDefinitionStreamableHandle.IsValid() && bAutoLoadItemDefinition)
			{
				LoadItemDefinition(ItemDefinitionSoft);
			}
		}
	}
}

void UItemViewModel::SetItemDefinition(const UItemDefinition* ItemDefinition)
{
	if (ItemDefinition && ItemDefinition != ItemDefinitionSoft.Get())
	{
		Internal_SetItemDefinition(ItemDefinition);
	}
}

bool UItemViewModel::IsAtMaxQuantity() const
{
	if (ItemInstanceViewModel)
	{
		return ItemInstanceViewModel->GetQuantity() >= MaxQuantity;
	}
	
	return true;
}

void UItemViewModel::OnItemInstanceSet()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsAtMaxQuantity);
}

void UItemViewModel::OnItemDefinitionSet(const UItemDefinition* ItemDefinition)
{
	SetItemName(ItemDefinition->ItemName);
	if (const FItemFragment_UI* UIFrag = ItemDefinition->FindFragmentByType<FItemFragment_UI>())
	{
		SetDescription(UIFrag->Description);
		SetIcon(UIFrag->Icon);
	}
		
	if (ItemInstanceViewModel)
	{
		if (const UItemContainer* ItemContainer = ItemInstanceViewModel->GetItemInstance().GetItemContainer())
		{
			SetMaxQuantity(ItemContainer->GetItemQuantityLimit(ItemInstanceViewModel->GetItemInstance().GetItem()));
		}
	}
	else
	{
		SetMaxQuantity(0);
	}
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsAtMaxQuantity);
}

void UItemViewModel::SetItemName(FText InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ItemName, InValue);
}

void UItemViewModel::SetDescription(FText InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Description, InValue);
}

void UItemViewModel::SetIcon(const TSoftObjectPtr<UTexture2D>& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Icon, InValue);
}

void UItemViewModel::SetMaxQuantity(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxQuantity, InValue);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsAtMaxQuantity);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CanHaveMaxQuantityGreaterThanOne);
}

void UItemViewModel::Internal_SetItemDefinition(const UItemDefinition* ItemDefinition)
{
	ItemDefinitionSoft = ItemDefinition->GetPathName();
	OnItemDefinitionSet(ItemDefinition);
	K2_OnItemDefinitionSet(ItemDefinition);
}

void UItemViewModel::Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UItemDefinition> Definition)
{
	if (const UItemDefinition* ItemDefinition = Definition.Get())
	{
		Internal_SetItemDefinition(ItemDefinition);
	}
	
	if (bAutoUnloadItemDefinition)
	{
		ItemDefinitionStreamableHandle.Reset();
	}
}
