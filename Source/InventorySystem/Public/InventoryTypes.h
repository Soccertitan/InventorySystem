// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"

#include "InventoryTypes.generated.h"

class UItemSet;
class UItemContainer;
class UInventoryManagerComponent;
class UItemDefinition;
struct FItem;
struct FGameplayTagStackContainer;
struct FItemInstance;

/**
 * Defines limitations for the quantity of an item.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FQuantityLimit
{
	GENERATED_BODY()

	FQuantityLimit(){}

	/** Return the max quantity or MAX_Int32 if unlimited. */
	int32 GetMaxQuantity() const;

	void SetMaxQuantity(int32 Quantity);
	void SetLimitQuantity(bool bValue = false);
	
private:
	/** Limits the quantity. */
	UPROPERTY(EditAnywhere, Meta = (InlineEditConditionToggle), Category = "Item")
	bool bLimitQuantity = false;

	/** The maximum quantity allowed. */
	UPROPERTY(EditAnywhere, Meta = (EditCondition="bLimitQuantity", ClampMin = 1), Category = "Item", DisplayName = "LimitQuantity")
	int32 MaxQuantity = 1;
};

UENUM(BlueprintType)
enum class EAddItemResult : uint8
{
	NoItemsAdded UMETA(DIsplayName = "No items added"),
	SomeItemsAdded UMETA(DisplayName = "Some items added"),
	AllItemsAdded UMETA(DisplayName = "All items added")
};

/**
 * Describes how to add an item to the ItemContainer or modifies an existing one.
 * Initialize it with either an ItemInstance or a TInstancedStruct<FItem> along with the Quantity that should be
 * added to the item.
 */
USTRUCT()
struct INVENTORYSYSTEM_API FAddItemPlanEntry
{
	GENERATED_BODY()

	FAddItemPlanEntry(){}
	FAddItemPlanEntry(FItemInstance* InItemInstance, int32 Quantity) :
		ItemInstance(InItemInstance),
		QuantityToAdd(Quantity)
		{}
	FAddItemPlanEntry(TInstancedStruct<FItem> InItem, int32 Quantity) :
		NewItem(InItem),
		QuantityToAdd(Quantity)
		{}

	/**
	 * The Item to modify.
	 */
	FItemInstance* ItemInstance = nullptr;

	// If the ItemInstancePtr is invalid, add the NewItem instead.
	TInstancedStruct<FItem> NewItem;
	
	// The amount that was added.
	UPROPERTY()
	int32 QuantityToAdd = 0;

	// Returns true if (QuantityToAdd > 0) and either the ItemInstancePtr or NewItem is valid.
	bool IsValid() const;
};

/**
 * Represents a collection of AddItemPlanEntries and expected results for adding items.
 */
USTRUCT()
struct INVENTORYSYSTEM_API FAddItemPlan
{
	GENERATED_BODY()

	FAddItemPlan(){}
	FAddItemPlan(int32 InItemQuantity) : AmountToGive(InItemQuantity), AmountGiven(0) {}

	// The amount of the item that we tried to add
	int32 GetAmountToGive() const { return AmountToGive; }
	// The amount of the item that will actually be added in the end. Maybe we tried adding 10 items, but only 8 could be added because of capacity/weight
	int32 GetAmountGiven() const { return AmountGiven; }
	EAddItemResult GetResult() const { return Result; }

	// Describes the reason for failure to add all items.
	UPROPERTY()
	FGameplayTag Error;

	/** Returns true if each AddItemPlanEntry has a quantity to add greater than 0. */
	bool IsValid() const;
	
	/** Adds a new Entry to the Entries array. Will properly update the AmountGiven. */
	void AddEntry(const FAddItemPlanEntry& InEntry);

	const TArray<FAddItemPlanEntry>& GetEntries() const;

private:
	// The amount of the item that we tried to add
	UPROPERTY()
	int32 AmountToGive = 0;

	// The amount of the item that will actually be added in the end. Maybe we tried adding 10 items, but only 8 could be added because of capacity/weight
	UPROPERTY()
	int32 AmountGiven = 0;

	// The result
	UPROPERTY()
	EAddItemResult Result = EAddItemResult::NoItemsAdded;

	UPROPERTY()
	TArray<FAddItemPlanEntry> Entries;

	// Adds to the AmountGiven and updates the EItemAddResult.
	void UpdateAmountGiven(int32 NewValue);
};

/**
 * Represents the items added to an ItemContainer.
 */
USTRUCT(BlueprintType)
struct FAddItemPlanResult
{
	GENERATED_BODY()
	FAddItemPlanResult(){}
	FAddItemPlanResult(const FAddItemPlan& InPlan, const TArray<FGuid>& InItemGuids);

	/** The ItemGuids of the items added or modified in the ItemManager. */
	UPROPERTY(BlueprintReadOnly)
	TArray<FGuid> ItemGuids;

	/** The amount of the item that we tried to add */
	UPROPERTY(BlueprintReadOnly)
	int32 AmountToGive = 0;

	/**
	 * The amount of the item that was actually added in the end. Maybe we tried adding 10 items, but only 8 could be
	 * added because of capacity, weight, etc...
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 AmountGiven = 0;

	// The result
	UPROPERTY(BlueprintReadOnly)
	EAddItemResult Result = EAddItemResult::NoItemsAdded;

	// Describes the reason for failure to add all items.
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Error;
};

//------------------------------------------------------------------------------
// GameplayTagStack
//------------------------------------------------------------------------------

/**
 * Represents a single gameplay tag and a count.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FGameplayTagStack
{
	GENERATED_BODY()

	FGameplayTagStack()
	{
	}

	FGameplayTagStack(FGameplayTag InTag, int32 InCount)
		: Tag(InTag),
		  Count(InCount)
	{
	}

	FString ToDebugString() const;

	bool operator==(const FGameplayTagStack& Other) const
	{
		return Tag == Other.Tag && Count == Other.Count;
	}

	bool operator!=(const FGameplayTagStack& Other) const
	{
		return Tag != Other.Tag || Count != Other.Count;
	}

	const FGameplayTag& GetTag() const;
	int32 GetCount() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), SaveGame)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), SaveGame)
	int32 Count = 0;

	friend FGameplayTagStackContainer;
};

/**
 * GameplayTagStackContainer
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FGameplayTagStackContainer
{
	GENERATED_BODY()

	FGameplayTagStackContainer() {}

	/** Add stacks to a tag. */
	void AddStack(FGameplayTag Tag, int32 DeltaCount);

	/** Subtracts stacks from a tag. */
	void SubtractStack(FGameplayTag Tag, int32 DeltaCount);

	/** Removes the Tag entirely */
	void RemoveStack(FGameplayTag Tag);

	/** Return the stack count for a tag, or 0 if the tag is not present. */
	int32 GetStackCount(FGameplayTag Tag) const;

	/** Returns a const reference to the current GameplayTagStackContainer. */
	const TArray<FGameplayTagStack>& GetItems() const;

	/** Return true if there is at least one count of a tag. */
	bool ContainsTag(FGameplayTag Tag, bool bExactMatch = false) const;

	/** Empties all stats in this container. */
	void Empty();

	FString ToDebugString() const;

	bool operator ==(const FGameplayTagStackContainer& Other) const
	{
		return Items == Other.Items;
	}

	bool operator !=(const FGameplayTagStackContainer& Other) const
	{
		return !(*this == Other);
	}

private:
	UPROPERTY(EditAnywhere, SaveGame)
	TArray<FGameplayTagStack> Items;
};

/**
 * Used by the ItemManager to create an ItemContainer and to try and add items to it.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FStartupItems
{
	GENERATED_BODY()

	FStartupItems () {}
	FStartupItems(TSubclassOf<UItemContainer> InContainerClass)
		:ItemContainerClass(InContainerClass) {}

	// The ItemContainer class to create.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UItemContainer> ItemContainerClass;

	// The items to try and add to the container.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UItemSet>> ItemSets;
};
