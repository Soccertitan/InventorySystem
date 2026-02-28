// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "StructUtils/InstancedStruct.h"

#include "InventoryFastTypes.generated.h"

class UItemContainer;
class UInventoryManagerComponent;

//--------------------------------------------------------------------------------------------------------------------
// ItemInstance Fast Array Serializer
//--------------------------------------------------------------------------------------------------------------------

/**
 * FastArraySerializerItem wrapper for an Item.
 * 
 * This is needed to avoid data slicing an Item as a direct FastArraySerializerItem, it needs to be handled with a TInstancedStruct.
 * TInstancedStruct does not support FastArray Serialization. Therefore, we need to wrap it in our own type.
 * 
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemInstance : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
	FGuid GetGuid() const { return Guid; }
	UInventoryManagerComponent* GetInventoryManagerComponent() const;
	UItemContainer* GetItemContainer() const { return WeakItemContainer.Get(); }
	UItemContainer* GetMovedToItemContainer() const { return WeakMovedToItemContainer.Get(); }
	bool IsValid() const;
	
	/** You must manually call this when the ItemInstance has been modified. */
	void MarkItemDirty();

	//~ Begin of FFastArraySerializerItem
	void PostReplicatedAdd(const struct FItemInstanceContainer& InSerializer);
	void PostReplicatedChange(const FItemInstanceContainer& InSerializer);
	void PreReplicatedRemove(const FItemInstanceContainer& InSerializer);
	//~ End of FFastArraySerializerItem

	/** A const reference of the Item. */
	const TInstancedStruct<FItem>& GetItem() const;
	/** A pointer to the Item. */
	TInstancedStruct<FItem>* GetItemPtr();
	/** Holds the previous value of the item during a broadcast event. */
	const TInstancedStruct<FItem>& GetPreReplicatedItem() const;
	int32 GetQuantity() const;
	
	friend bool operator==(const FItemInstance& X, const FItemInstance& Y)
	{
		return X.GetGuid() == Y.GetGuid();
	}
	
	friend bool operator!=(const FItemInstance& X, const FItemInstance& Y)
	{
		return X.GetGuid() != Y.GetGuid();
	}

private:

	/** The unique identifier for this Item. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	FGuid Guid;

	/* The replicated Item. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	TInstancedStruct<FItem> Item;
	
	/** The number of item instances. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	int32 Quantity = 0;

	/** The ItemContainer that holds this item. */
	UPROPERTY(NotReplicated)
	TWeakObjectPtr<UItemContainer> WeakItemContainer;

	/** The ItemContainer the Item was moved into. Only valid on Server when the ItemInstance is removed from this ItemContainer. */
	UPROPERTY(NotReplicated)
	TWeakObjectPtr<UItemContainer> WeakMovedToItemContainer;

	/* A copy of the Item which we use as a lookup for the previous values of changed properties. */
	UPROPERTY(NotReplicated, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TInstancedStruct<FItem> PreReplicatedChangeItem;

	friend UItemContainer;
	friend UInventoryManagerComponent;
	friend struct FItemInstanceContainer;
};

/**
 * FastArraySerializer for ItemInstances.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemInstanceContainer : public FFastArraySerializer
{
    GENERATED_BODY()

    FItemInstanceContainer(){}

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
    {
        return FastArrayDeltaSerialize<FItemInstance, FItemInstanceContainer>(Items, DeltaParams, *this);
    }

    /** Adds an Item to the list. */
    void AddItem(const FGuid& Guid, const TInstancedStruct<FItem>& Item, const int32 Quantity);

    /** Removes an Item from the list. */
    void RemoveItem(const FGuid& Guid);

    /** Returns a const reference of all the Items within the container. */
    const TArray<FItemInstance>& GetItems() const;

	/** Returns a pointer to an Item. */
	FItemInstance* GetItem(const FGuid& Guid) const;

    /** Returns the number of Items in the container. */
    int32 GetNum() const;

	UItemContainer* GetOwner() const { return Owner; }

	/** Removes all items from this container. */
	void Reset();

private:
	UPROPERTY()
	TArray<FItemInstance> Items;

	UPROPERTY(NotReplicated)
	TObjectPtr<UItemContainer> Owner;

	friend UInventoryManagerComponent;
	friend UItemContainer;
	friend struct FItemInstance;
};

template<>
struct TStructOpsTypeTraits<FItemInstanceContainer> : public TStructOpsTypeTraitsBase2<FItemInstanceContainer>
{
    enum
    {
        WithNetDeltaSerializer = true
    };
};


//--------------------------------------------------------------------------------------------------------------------
// ItemContainer FastArray
//--------------------------------------------------------------------------------------------------------------------

/**
 * Holds an ItemContainer.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemContainerInstance : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FString ToDebugString() const;

	UItemContainer* GetItemContainer() const { return ItemContainer; }

	//~ Begin of FFastArraySerializerItem
	void PostReplicatedAdd(const struct FItemContainerInstanceContainer& InSerializer);
	void PreReplicatedRemove(const struct FItemContainerInstanceContainer& InSerializer);
	//~ End of FFastArraySerializerItem

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	TObjectPtr<UItemContainer> ItemContainer = nullptr;

	friend struct FItemContainerInstanceContainer;

	friend bool operator==(const FItemContainerInstance& X, const FItemContainerInstance& Y)
	{
		return X.ItemContainer == Y.ItemContainer;
	}
	friend bool operator!=(const FItemContainerInstance& X, const FItemContainerInstance& Y)
	{
		return X.ItemContainer != Y.ItemContainer;
	}
};

/**
 * Holds ItemContainerInstances
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemContainerInstanceContainer : public FFastArraySerializer
{
	GENERATED_BODY()
	
	FItemContainerInstanceContainer() {}
	
	/** Adds a new ItemContainer. */
	void AddItemContainer(UItemContainer* ItemContainer);

	/**
	 * Removes the ItemContainer.
	 */
	void RemoveItemContainer(UItemContainer* ItemContainer);

	/**
	 * Removes the ItemContainer by GameplayTag.
	 */
	void RemoveItemContainer(const FGameplayTag& ItemContainerTag);
	
	/** Gets a const reference to the ItemContainers. */
	const TArray<FItemContainerInstance>& GetItems() const;

	UItemContainer* GetItemContainerByTag(const FGameplayTag& ItemContainerTag) const;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FItemContainerInstance, FItemContainerInstanceContainer>(Items, DeltaParms, *this);
	}

private:
	/** Replicated array of ItemContainers. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	TArray<FItemContainerInstance> Items;

	/** Component that owns this container. */
	UPROPERTY(NotReplicated)
	TObjectPtr<UInventoryManagerComponent> Owner;

	friend UInventoryManagerComponent;
	friend struct FItemContainerInstance;
};

template <>
struct TStructOpsTypeTraits<FItemContainerInstanceContainer> : public TStructOpsTypeTraitsBase2<FItemContainerInstanceContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
