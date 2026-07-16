// Copyright Soccertitan 2025


#include "InventorySettings.h"

#include "ItemContainer/ItemContainer.h"
#include "InventorySystem.h"
#include "NativeGameplayTags.h"
#include "Engine/AssetManager.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"

namespace DefaultTag
{
	UE_DEFINE_GAMEPLAY_TAG_STATIC(ItemContainer_Default, "ItemContainer.Default")
}

UInventorySettings::UInventorySettings()
{
	ItemContainerTag = DefaultTag::ItemContainer_Default;
	ItemContainerClass = UItemContainer::StaticClass();
	ItemInstanceViewModelClass = UItemInstanceViewModel::StaticClass();
}

FName UInventorySettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

FGameplayTag UInventorySettings::GetItemContainerTag()
{
	const UInventorySettings* Settings = GetDefault<UInventorySettings>();

	if (!Settings->ItemContainerTag.IsValid())
	{
		UE_LOG(LogInventorySystem, Error, TEXT("UInventorySettings.ItemContainerTag is not valid. "
			"Set a value in the project settings."));
	}

	return Settings->ItemContainerTag;
}

TSubclassOf<UItemContainer> UInventorySettings::GetItemContainerClass()
{
	const UInventorySettings* Settings = GetDefault<UInventorySettings>();

	if (!Settings->ItemContainerClass)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("UInventorySettings.ItemContainerClass is not valid. "
			"Set a value in the project settings."));
	}

	if (!Settings->ItemContainerClass.Get())
	{
		UAssetManager::Get().LoadAssetList({Settings->ItemContainerClass.ToSoftObjectPath()})->WaitUntilComplete();
	}

	return Settings->ItemContainerClass.Get();
}

TSubclassOf<UItemInstanceViewModel> UInventorySettings::GetItemInstanceViewModelClass()
{
	const UInventorySettings* Settings = GetDefault<UInventorySettings>();

	if (!Settings->ItemInstanceViewModelClass)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("UInventorySettings.ItemInstanceViewModelClass is not valid. "
			"Set a value in the project settings."));
	}

	if (!Settings->ItemInstanceViewModelClass.Get())
	{
		UAssetManager::Get().LoadAssetList({Settings->ItemInstanceViewModelClass.ToSoftObjectPath()})->WaitUntilComplete();
	}

	return Settings->ItemInstanceViewModelClass.Get();
}


