// Copyright Soccertitan 2025


#include "InventoryBlueprintFunctionLibrary.h"

#include "Item/ItemDefinition.h"
#include "InventoryManagerComponent.h"
#include "InventorySystemInterface.h"
#include "Engine/AssetManager.h"

UInventoryManagerComponent* UInventoryBlueprintFunctionLibrary::GetInventoryManagerComponent(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	if (Actor->Implements<UInventorySystemInterface>())
	{
		return IInventorySystemInterface::Execute_GetInventoryManagerComponent(Actor);
	}

	return Actor->FindComponentByClass<UInventoryManagerComponent>();
}

bool UInventoryBlueprintFunctionLibrary::IsItemInstanceValid(const FItemInstance& ItemInstance)
{
	return ItemInstance.IsValid();
}

const UItemDefinition* UInventoryBlueprintFunctionLibrary::GetItemDefinition(const TInstancedStruct<FItem>& Item)
{
	if (Item.IsValid())
	{
		if (!Item.Get<FItem>().GetItemDefinition().Get())
		{
			UAssetManager::Get().LoadAssetList(
				{Item.Get<FItem>().GetItemDefinition().ToSoftObjectPath()})->WaitUntilComplete();
		}
		return Item.Get<FItem>().GetItemDefinition().Get();
	}
	return nullptr;
}

TInstancedStruct<FItemShard> UInventoryBlueprintFunctionLibrary::K2_FindItemShard(const TInstancedStruct<FItem>& Item, const UScriptStruct* FragmentType)
{
	if (Item.IsValid() && FragmentType)
	{
		const FItem* ItemPtr = Item.GetPtr<FItem>();
		return ItemPtr->FindShardByScriptStruct(FragmentType);
	}
	return TInstancedStruct<FItemShard>();
}

TInstancedStruct<FItemFragment> UInventoryBlueprintFunctionLibrary::K2_FindItemFragment(const UItemDefinition* ItemDefinition, const UScriptStruct* FragmentType)
{
	if (ItemDefinition && FragmentType)
	{
		for (const TInstancedStruct<FItemFragment>& Fragment : ItemDefinition->Fragments)
		{
			if (Fragment.IsValid() && Fragment.GetScriptStruct()->IsChildOf(FragmentType))
			{
				return Fragment;
			}
		}
	}
	return TInstancedStruct<FItemFragment>();
}
