// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"

#include "Item.generated.h"

class UItemContainer;
class UInventoryManagerComponent;

/**
 * Contains custom state information for an item.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemShard
{
	GENERATED_BODY()

	FItemShard() {}
	virtual ~FItemShard() {}

	virtual bool IsMatching(const TInstancedStruct<FItemShard>& Shard) const {return true;}
};

/**
 * The base representation of an Item. This can be extended with child structs.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItem
{
	GENERATED_BODY()

	FItem();
	virtual ~FItem(){}

	TSoftObjectPtr<UItemDefinition> GetItemDefinition() const { return ItemDefinition; }

	/** Tags representing various stats about this item, such as level, use count, remaining ammo, etc... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FGameplayTagStackContainer GameplayTagStackContainer;

	/** Returns true if this Item has the same ItemDefinition, GameplayTagStackContainer, and ItemFragments as the Item. */
	virtual bool IsMatching(const TInstancedStruct<FItem>& Item) const;

	template<typename T> requires std::derived_from<T, FItemShard>
	const T* FindShardByType() const;
	template<typename T> requires std::derived_from<T, FItemShard>
	T* FindMutableShardByType();

	/** Returns a copy of the*/
	TInstancedStruct<FItemShard> FindShardByScriptStruct(const UScriptStruct* Struct) const;
	
protected:
	/** Extends an item's capabilities. Added through ItemDefinitionFragments on creation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, meta = (FullyExpand=true, StructTypeConst), EditFixedSize)
	TArray<TInstancedStruct<FItemShard>> Shards;

	/** Called when the item is created. */
	virtual void Initialize(const UItemDefinition* InItemDefinition);

private:

	/** The static data representing this item. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true), SaveGame)
	TSoftObjectPtr<UItemDefinition> ItemDefinition;

	friend UInventoryManagerComponent;
	friend struct FItemFragment;

	/**
	 * Iterates through self and Item's shards. And calls IsMatching on each one.
	 */
	bool AreShardsEqual(const TInstancedStruct<FItem>& Item) const;
};

/**
 * @return A const pointer to the first shard that matches the type.
 */
template <typename T> requires std::derived_from<T, FItemShard>
const T* FItem::FindShardByType() const
{
	for (const TInstancedStruct<FItemShard>& Shard : Shards)
	{
		if (const T* Ptr = Shard.GetPtr<T>())
		{
			return Ptr;
		}
	}
	return nullptr;
}

/**
 * @return A mutable pointer to the first shard that matches the type.
 */
template <typename T> requires std::derived_from<T, FItemShard>
T* FItem::FindMutableShardByType()
{
	for (TInstancedStruct<FItemShard>& Shard : Shards)
	{
		if (T* Ptr = Shard.GetMutablePtr<T>())
		{
			return Ptr;
		}
	}
	return nullptr;
}
