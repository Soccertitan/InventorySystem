// Copyright Soccertitan


#include "UI/ViewModel/ItemViewModel.h"

#include "Item/ItemDefinition.h"
#include "InventoryFastTypes.h"
#include "InventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "ItemContainer/ItemContainer.h"
#include "Item/Fragment/ItemFragment_UI.h"
#include "UI/ItemViewModelInterface.h"

UItemViewModel::UItemViewModel()
{
	Bundles.Add("UI");
}

void UItemViewModel::SetItemInstance(const FItemInstance& ItemInstance)
{
	if (ItemInstance.IsValid())
	{
		UE_MVVM_SET_PROPERTY_VALUE(bItemValid, true);

		bool bShouldLoadItemDefinition = false;
		if (Guid != ItemInstance.GetGuid())
		{
			ItemDefinitionStreamableHandle.Reset();
			bShouldLoadItemDefinition = true;
		}

		Guid = ItemInstance.GetGuid();
		Item = ItemInstance.Item;
		ItemContainer = ItemInstance.GetItemContainer();
		InventoryManagerComponent = ItemContainer->GetInventoryManagerComponent();
		OnItemSet(ItemInstance);
		K2_OnItemSet(ItemInstance);

		if (bShouldLoadItemDefinition && bAutoLoadItemDefinition)
		{
			LoadItemDefinition();
		}
	}
	else
	{
		ItemDefinitionStreamableHandle.Reset();
		UE_MVVM_SET_PROPERTY_VALUE(bItemValid, false);
	}
}

UUserWidget* UItemViewModel::CreateItemDetailsWidget(APlayerController* OwningPlayer, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		if (ItemWidgetClass.Get())
		{
			UAssetManager::Get().LoadAssetList({ItemWidgetClass.ToSoftObjectPath()})->WaitUntilComplete();
		}
		WidgetClass = ItemWidgetClass.Get();
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

void UItemViewModel::LoadItemDefinition()
{
	if (const FItem* ItemPtr = Item.GetPtr<FItem>())
	{
		FPrimaryAssetId AssetId = UAssetManager::Get().GetPrimaryAssetIdForPath(
		   ItemPtr->GetItemDefinition().ToSoftObjectPath());
	
		if (AssetId.IsValid())
		{
			FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this,
				&UItemViewModel::Internal_OnItemDefinitionLoaded, ItemPtr->GetItemDefinition());
			ItemDefinitionStreamableHandle = UAssetManager::Get().PreloadPrimaryAssets(
			   {AssetId}, Bundles, bLoadRecursive, Delegate);
		
			UE_MVVM_SET_PROPERTY_VALUE(bItemDefinitionLoading, ItemDefinitionStreamableHandle->IsLoadingInProgress());
		}
	}
}

void UItemViewModel::OnItemSet(const FItemInstance& ItemInstance)
{
	const FItem* ItemPtr = ItemInstance.Item.GetPtr<FItem>();
	SetQuantity(ItemPtr->GetQuantity());
}

void UItemViewModel::OnItemDefinitionLoaded(const UItemDefinition* ItemDefinition)
{
	const FItemFragment_UI* UIFrag = ItemDefinition->FindFragmentByType<FItemFragment_UI>();
	SetItemName(UIFrag->ItemName);
	SetDescription(UIFrag->Description);
	SetIcon(UIFrag->Icon.Get());
	SetMaxQuantity(GetItemContainer()->GetItemQuantityLimit(GetItem()));
}

void UItemViewModel::SetItemName(FText InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ItemName, InValue);
}

void UItemViewModel::SetDescription(FText InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Description, InValue);
}

void UItemViewModel::SetIcon(UTexture2D* InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Icon, InValue);
}

void UItemViewModel::SetQuantity(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Quantity, InValue);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsAtMaxQuantity);
}

void UItemViewModel::SetMaxQuantity(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxQuantity, InValue);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsAtMaxQuantity);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CanHaveMaxQuantityGreaterThanOne);
}

void UItemViewModel::Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UItemDefinition> ItemDefinition)
{
	UE_MVVM_SET_PROPERTY_VALUE(bItemDefinitionLoading, false);
	ItemWidgetClass = ItemDefinition.Get()->FindFragmentByType<FItemFragment_UI>()->WidgetClass;
	OnItemDefinitionLoaded(ItemDefinition.Get());
	K2_OnItemDefinitionLoaded(ItemDefinition.Get());
	ItemDefinitionStreamableHandle.Reset();
}
