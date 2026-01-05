// Copyright Soccertitan 2025


#include "Item/Item.h"

#include "Item/ItemDefinition.h"


FItem::FItem()
{
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

	for (int32 Index = 0; Index < Shards.Num(); Index++)
	{
		if (!Shards[Index].Get<FItemShard>().IsMatching(TestItemPtr->Shards[Index]))
		{
			return false;
		}
	}

	return true;
}

TInstancedStruct<FItemShard> FItem::FindShardByScriptStruct(const UScriptStruct* Struct) const
{
	for (const TInstancedStruct<FItemShard>& Shard : Shards)
	{
		if (Shard.IsValid() && Shard.GetScriptStruct()->IsChildOf(Struct))
		{
			return Shard;
		}
	}
	return TInstancedStruct<FItemShard>();
}

void FItem::Initialize(const UItemDefinition* InItemDefinition)
{
	ItemDefinition = InItemDefinition->GetPathName();

	for (const TTuple<FGameplayTag, int>& Pair : InItemDefinition->DefaultStats)
	{
		GameplayTagStackContainer.AddStack(Pair.Key, Pair.Value);
	}
}
