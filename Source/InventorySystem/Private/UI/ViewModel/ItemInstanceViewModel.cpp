// Copyright 2025 Soccertitan


#include "UI/ViewModel/ItemInstanceViewModel.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "InventoryManagerComponent.h"
#include "InventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "Item/ItemDefinition.h"
#include "Item/Fragment/ItemFragment_UI.h"
#include "ItemContainer/ItemContainer.h"
#include "UI/ItemViewModelInterface.h"

UItemInstanceViewModel::UItemInstanceViewModel()
{
	Bundles.Add("UI");
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
		OnItemInstanceSet();
		K2_OnItemInstanceSet();

		if (bShouldLoadItemDefinition && bAutoLoadItemDefinition)
		{
			LoadItemDefinition();
		}
	}
}

UUserWidget* UItemInstanceViewModel::CreateItemDetailsWidget(APlayerController* OwningPlayer, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		if (const UItemDefinition* ItemDefinition = UInventoryBlueprintFunctionLibrary::GetItemDefinition(ItemInstance.GetItem()))
		{
			if (const FItemFragment_UI* Fragment = ItemDefinition->FindFragmentByType<FItemFragment_UI>())
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
	if (const FItem* ItemPtr = ItemInstance.GetItem().GetPtr<FItem>())
	{
		FPrimaryAssetId AssetId = UAssetManager::Get().GetPrimaryAssetIdForPath(
		   ItemPtr->GetItemDefinition().ToSoftObjectPath());
	
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
	if (const UItemDefinition* ItemDefinition = UInventoryBlueprintFunctionLibrary::GetItemDefinition(ItemInstance.GetItem()))
	{
		SetItemName(ItemDefinition->ItemName);
		if (const FItemFragment_UI* UIFrag = ItemDefinition->FindFragmentByType<FItemFragment_UI>())
		{
			SetDescription(UIFrag->Description);
			SetIcon(UIFrag->Icon);
		}
		if (ItemInstance.GetItemContainer())
		{
			SetMaxQuantity(ItemInstance.GetItemContainer()->GetItemQuantityLimit(ItemInstance.GetItem()));
		}
		else
		{
			SetMaxQuantity(0);
		}
		OnItemDefinitionLoaded(ItemDefinition);
		K2_OnItemDefinitionLoaded(ItemDefinition);
		
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(OnViewModelInitialized);
	}
	
	if (bAutoUnloadItemDefinition)
	{
		ItemDefinitionStreamableHandle.Reset();
	}
}
