// Copyright Soccertitan 2025


#include "UI/ViewModel/ItemViewModel.h"

#include "Item/ItemDefinition.h"
#include "InventoryFastTypes.h"
#include "InventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "Item/Fragment/ItemFragment_UI.h"
#include "UI/ItemViewModelInterface.h"

UItemViewModel::UItemViewModel()
{
	Bundles.Add("UI");
}

void UItemViewModel::SetItem(const TInstancedStruct<FItem>& InItem)
{
	if (InItem.IsValid())
	{
		bool bShouldLoadItemDefinition = false;
		
		if (CachedItem.IsValid())
		{
			if (InItem.Get().GetItemDefinition() != CachedItem.Get().GetItemDefinition())
			{
				ItemDefinitionStreamableHandle.Reset();
				bShouldLoadItemDefinition = true;
			}
		}
		else
		{
			bShouldLoadItemDefinition = true;
		}
		
		CachedItem = InItem;
		
		OnItemSet(CachedItem);
		K2_OnItemSet(CachedItem);
		
		if (bShouldLoadItemDefinition && bAutoLoadItemDefinition)
		{
			LoadItemDefinition();
		}
	}
	else
	{
		ItemDefinitionStreamableHandle.Reset();
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
	if (const FItem* ItemPtr = CachedItem.GetPtr<FItem>())
	{
		FPrimaryAssetId AssetId = UAssetManager::Get().GetPrimaryAssetIdForPath(
		   ItemPtr->GetItemDefinition().ToSoftObjectPath());
	
		if (AssetId.IsValid())
		{
			FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this,
				&UItemViewModel::Internal_OnItemDefinitionLoaded, ItemPtr->GetItemDefinition());
			ItemDefinitionStreamableHandle = UAssetManager::Get().PreloadPrimaryAssets(
			   {AssetId}, Bundles, bLoadRecursive, Delegate);
		}
	}
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

void UItemViewModel::Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UItemDefinition> ItemDefinition)
{
	const FItemFragment_UI* UIFrag = ItemDefinition->FindFragmentByType<FItemFragment_UI>();
	ItemWidgetClass = UIFrag->WidgetClass;
	SetItemName(UIFrag->ItemName);
	SetDescription(UIFrag->Description);
	SetIcon(UIFrag->Icon.Get());

	OnItemDefinitionLoaded(ItemDefinition.Get());
	K2_OnItemDefinitionLoaded(ItemDefinition.Get());
	
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(OnViewModelInitialized);

	ItemDefinitionStreamableHandle.Reset();
}
