// Copyright Soccertitan 2025


#include "InventorySettings.h"

#include "ItemContainer/ItemContainer.h"
#include "InventorySystem.h"
#include "NativeGameplayTags.h"
#include "Engine/AssetManager.h"

namespace DefaultTag
{
	UE_DEFINE_GAMEPLAY_TAG_STATIC(ItemContainer_Default, "ItemContainer.Default")
}

UInventorySettings::UInventorySettings()
{
	DefaultItemContainerTag = DefaultTag::ItemContainer_Default;
	DefaultItemContainerClass = UItemContainer::StaticClass();
}

FName UInventorySettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

FGameplayTag UInventorySettings::GetDefaultItemContainerTag()
{
	const UInventorySettings* Settings = GetDefault<UInventorySettings>();

	if (!Settings->DefaultItemContainerTag.IsValid())
	{
		UE_LOG(LogInventorySystem, Error, TEXT("UInventorySettings.DefaultItemContainerTag is not valid. "
			"Set a value in the project settings."));
	}

	return Settings->DefaultItemContainerTag;
}

TSubclassOf<UItemContainer> UInventorySettings::GetDefaultItemContainerClass()
{
	const UInventorySettings* Settings = GetDefault<UInventorySettings>();

	if (!Settings->DefaultItemContainerClass)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("UInventorySettings.DefaultItemContainerClass is not valid. "
			"Set a value in the project settings."));
	}

	if (!Settings->DefaultItemContainerClass.Get())
	{
		UAssetManager::Get().LoadAssetList({Settings->DefaultItemContainerClass.ToSoftObjectPath()})->WaitUntilComplete();
	}

	return Settings->DefaultItemContainerClass.Get();
}


