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

FItemInstanceHandle UInventoryBlueprintFunctionLibrary::GetHandle(const FItemInstance& ItemInstance)
{
	return FItemInstanceHandle(ItemInstance);
}

bool UInventoryBlueprintFunctionLibrary::IsItemInstanceHandleValid(const FItemInstanceHandle& Handle)
{
	return Handle.IsValid();
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

TInstancedStruct<FItemFragment> UInventoryBlueprintFunctionLibrary::K2_FindItemFragment(const TInstancedStruct<FItem>& Item, const UScriptStruct* FragmentType)
{
	if (Item.IsValid() && FragmentType)
	{
		const FItem* ItemPtr = Item.GetPtr<FItem>();
		return ItemPtr->FindFragmentByScriptStruct(FragmentType);
	}
	return TInstancedStruct<FItemFragment>();
}

TInstancedStruct<FItemDefinitionFragment> UInventoryBlueprintFunctionLibrary::K2_FindItemDefinitionFragment(const UItemDefinition* ItemDefinition, const UScriptStruct* FragmentType)
{
	if (ItemDefinition && FragmentType)
	{
		for (const TInstancedStruct<FItemDefinitionFragment>& Fragment : ItemDefinition->Fragments)
		{
			if (Fragment.IsValid() && Fragment.GetScriptStruct()->IsChildOf(FragmentType))
			{
				return Fragment;
			}
		}
	}
	return TInstancedStruct<FItemDefinitionFragment>();
}
