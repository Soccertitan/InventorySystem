// Copyright Soccertitan


#include "Item/Item.h"

#include "Item/ItemDefinition.h"


FItem::FItem()
{
	Quantity = 0;
}

bool FItem::IsMatching(const TInstancedStruct<FItem>& Item) const
{
	if (!Item.IsValid())
	{
		return false;
	}
	
	if (Item.GetPtr<FItem>()->ItemDefinition != ItemDefinition)
	{
		return false;
	}

	if (Item.GetPtr<FItem>()->GameplayTagStackContainer != GameplayTagStackContainer)
	{
		return false;
	}

	if (!AreShardsEqual(Item))
	{
		return false;
	}

	return true;
}

bool FItem::AreShardsEqual(const TInstancedStruct<FItem>& Item) const
{
	const FItem* TestItemPtr = Item.GetPtr<FItem>();

	if (Shards.Num() != TestItemPtr->Shards.Num())
	{
		return false;
	}

	for (int32 idx = 0; idx < Shards.Num(); idx++)
	{
		if (!Shards[idx].Get<FItemShard>().IsMatching(TestItemPtr->Shards[idx]))
		{
			return false;
		}
	}

	return true;
}

TInstancedStruct<FItemShard> FItem::FindShardByScriptStruct(const UScriptStruct* Struct) const
{
	for (const TInstancedStruct<FItemShard>& Fragment : Shards)
	{
		if (Fragment.IsValid() && Fragment.GetScriptStruct()->IsChildOf(Struct))
		{
			return Fragment;
		}
	}
	return TInstancedStruct<FItemShard>();
}

void FItem::Initialize(const UItemDefinition* InItemDefinition)
{
	ItemDefinition = ItemDefinition->GetPathName();

	for (const TTuple<FGameplayTag, int>& Pair : ItemDefinition->DefaultStats)
	{
		GameplayTagStackContainer.AddStack(Pair.Key, Pair.Value);
	}
}
