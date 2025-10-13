// Copyright Soccertitan


#include "InventoryFastTypes.h"

#include "InventoryManagerComponent.h"
#include "ItemContainer/ItemContainer.h"


//----------------------------------------------------------------------------------------
// ItemInstance
//----------------------------------------------------------------------------------------
UInventoryManagerComponent* FItemInstance::GetInventoryManagerComponent() const
{
	if (UItemContainer* ItemContainer = WeakItemContainer.Get())
	{
		return ItemContainer->GetInventoryManagerComponent();
	}
	return nullptr;
}

bool FItemInstance::IsValid() const
{
	if (!Guid.IsValid())
	{
		return false;
	}

	if (!Item.IsValid())
	{
		return false;
	}

	if (Item.Get().GetItemDefinition().IsNull())
	{
		return false;
	}

	if (!WeakItemContainer.Get())
	{
		return false;
	}

	return true;
}

void FItemInstance::PostReplicatedAdd(const FItemInstanceContainer& InSerializer)
{
	// Update our cached state.
	PreReplicatedChangeItem = Item;
	WeakItemContainer = InSerializer.Owner;
	InSerializer.Owner->Internal_OnItemAdded(*this);
}

void FItemInstance::PostReplicatedChange(const FItemInstanceContainer& InSerializer)
{
	InSerializer.Owner->Internal_OnItemChanged(*this);
	PreReplicatedChangeItem = Item;
}

void FItemInstance::PreReplicatedRemove(const FItemInstanceContainer& InSerializer)
{
	InSerializer.Owner->Internal_OnItemRemoved(*this);
}

//----------------------------------------------------------------------------------------
// ItemInstanceContainer
//----------------------------------------------------------------------------------------

void FItemInstanceContainer::AddItem(const FGuid& Guid, const TInstancedStruct<FItem> Item)
{
	check(Guid.IsValid());
	check(Item.IsValid() && !Item.Get().GetItemDefinition().IsNull());

	FItemInstance& NewItem = Items.AddDefaulted_GetRef();
	NewItem.Guid = Guid;
	NewItem.Item = Item;
	NewItem.PreReplicatedChangeItem = Item;
	NewItem.WeakItemContainer = Owner;

	Owner->Internal_OnItemAdded(NewItem);
	MarkItemDirty(NewItem);
}

void FItemInstanceContainer::RemoveItem(const FGuid& Guid)
{
	for (int32 i = Items.Num() - 1; i >= 0; i--)
	{
		if (Items[i].GetGuid() == Guid)
		{
			FItemInstance OldItem = Items[i];
			Items.RemoveAtSwap(i);
			Owner->Internal_OnItemRemoved(OldItem);
			MarkArrayDirty();
			return;
		}
	}
}

const TArray<FItemInstance>& FItemInstanceContainer::GetItems() const
{
	return Items;
}

FItemInstance* FItemInstanceContainer::GetItem(const FGuid& Guid) const
{
	if (Guid.IsValid())
	{
		for (const FItemInstance& Item : Items)
		{
			if (Item.GetGuid() == Guid)
			{
				return const_cast<FItemInstance*>(&Item);
			}
		}
	}
	return nullptr;
}

int32 FItemInstanceContainer::GetNum() const
{
	return Items.Num();
}

void FItemInstanceContainer::Reset()
{
	TArray<FItemInstance> TempEntries = Items;
	Items.Empty();
	for (FItemInstance& Entry : TempEntries)
	{
		Owner->Internal_OnItemRemoved(Entry);
	}
	MarkArrayDirty();
}

//--------------------------------------------------------------------------
// ItemContainerInstance
//--------------------------------------------------------------------------
FString FItemContainerInstance::ToDebugString() const
{
	return ItemContainer ? ItemContainer->GetFName().ToString() : TEXT("(none)");
}

void FItemContainerInstance::PostReplicatedAdd(const FItemContainerInstanceContainer& InSerializer)
{
	ItemContainer->Initialize();
	InSerializer.Owner->OnContainerAdded(*this);
}

void FItemContainerInstance::PreReplicatedRemove(const FItemContainerInstanceContainer& InSerializer)
{
	InSerializer.Owner->OnContainerRemoved(*this);
}

//----------------------------------------------------------------------------------------
// ItemContainerInstanceContainer
//----------------------------------------------------------------------------------------

void FItemContainerInstanceContainer::AddItemContainer(UItemContainer* ItemContainer, const FGameplayTag& InContainerTag)
{
	FItemContainerInstance& NewEntry = Items.AddDefaulted_GetRef();
	NewEntry.ItemContainer = ItemContainer;
	NewEntry.ItemContainerTag = InContainerTag;
	Owner->OnContainerAdded(NewEntry);
	MarkItemDirty(NewEntry);
}

void FItemContainerInstanceContainer::RemoveItemContainer(UItemContainer* ItemContainer)
{
	if (IsValid(ItemContainer))
	{
		for (auto EntryIt = Items.CreateIterator(); EntryIt; ++EntryIt)
		{
			FItemContainerInstance& Entry = *EntryIt;
			if (Entry.ItemContainer == ItemContainer)
			{
				FItemContainerInstance TempEntry(Entry);
				EntryIt.RemoveCurrentSwap();
				Owner->OnContainerRemoved(TempEntry);
				return;
			}
		}
	}
}

void FItemContainerInstanceContainer::RemoveItemContainer(const FGameplayTag& ItemContainerTag)
{
	if (ItemContainerTag.IsValid())
	{
		for (auto EntryIt = Items.CreateIterator(); EntryIt; ++EntryIt)
		{
			FItemContainerInstance& Entry = *EntryIt;
			if (Entry.ItemContainerTag == ItemContainerTag)
			{
				FItemContainerInstance TempEntry(Entry);
				EntryIt.RemoveCurrentSwap();
				Owner->OnContainerRemoved(TempEntry);
				return;
			}
		}
	}
}

const TArray<FItemContainerInstance>& FItemContainerInstanceContainer::GetItems() const
{
	return Items;
}

UItemContainer* FItemContainerInstanceContainer::GetItemContainerByTag(const FGameplayTag& ItemContainerTag) const
{
	for (const FItemContainerInstance& ItemContainerInstance : Items)
	{
		if (ItemContainerInstance.GetItemContainerTag() == ItemContainerTag)
		{
			return ItemContainerInstance.GetItemContainer();
		}
	}
	return nullptr;
}
