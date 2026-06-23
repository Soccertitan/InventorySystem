// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"

#include "Item.generated.h"

class UItemContainer;
class UInventoryManagerComponent;

/**
 * Custom data that is stateful information for an item.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemFragment
{
	GENERATED_BODY()

	FItemFragment() {}
	virtual ~FItemFragment() {}

	/** Returns true if the OtherFragment's StaticStruct equals this one. */
	virtual bool IsMatching(const TInstancedStruct<FItemFragment>& OtherFragment) const;
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
	virtual bool IsMatching(const TInstancedStruct<FItem>& OtherItem) const;

	template<typename T> requires std::derived_from<T, FItemFragment>
	const T* FindFragmentByType() const;
	template<typename T> requires std::derived_from<T, FItemFragment>
	T* FindMutableFragmentByType();

	/** Returns a copy of the*/
	TInstancedStruct<FItemFragment> FindFragmentByScriptStruct(const UScriptStruct* Struct) const;
	
protected:
	/** Extends an item's capabilities. Added through ItemDefinitionFragments on creation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, meta = (FullyExpand=true, StructTypeConst), EditFixedSize)
	TArray<TInstancedStruct<FItemFragment>> Fragments;

	/** Called when the item is created. */
	virtual void Initialize(const UItemDefinition* InItemDefinition);

private:
	/** The static data representing this item. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true), SaveGame)
	TSoftObjectPtr<UItemDefinition> ItemDefinition;

	friend UInventoryManagerComponent;

	/**
	 * Iterates through self and Item's Fragments. And calls IsMatching on each one.
	 */
	bool AreFragmentsEqual(const TInstancedStruct<FItem>& OtherItem) const;
};

/**
 * @return A const pointer to the first fragment that matches the type.
 */
template <typename T> requires std::derived_from<T, FItemFragment>
const T* FItem::FindFragmentByType() const
{
	for (const TInstancedStruct<FItemFragment>& Fragment : Fragments)
	{
		if (const T* Ptr = Fragment.GetPtr<T>())
		{
			return Ptr;
		}
	}
	return nullptr;
}

/**
 * @return A mutable pointer to the first Fragment that matches the type.
 */
template <typename T> requires std::derived_from<T, FItemFragment>
T* FItem::FindMutableFragmentByType()
{
	for (TInstancedStruct<FItemFragment>& Fragment : Fragments)
	{
		if (T* Ptr = Fragment.GetMutablePtr<T>())
		{
			return Ptr;
		}
	}
	return nullptr;
}
