// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "InventoryFastTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif
#include "ItemContainer.generated.h"

class UItemContainerRule_RemoveItem;
class UItemContainerRule_AddItem;
class UItemContainerRule_ItemQuantity;
class UItemContainerViewModel;

DECLARE_MULTICAST_DELEGATE_TwoParams(FItemContainerItemInstanceSignature, UItemContainer*, const FItemInstance&);

/**
 * A basic implementation for an ItemContainer that can hold any number of ItemInstances.
 * Designed to be subclassed to define how items are added and managed in the ItemContainer.
 */
UCLASS(Blueprintable, BlueprintType)
class INVENTORYSYSTEM_API UItemContainer : public UObject
{
	GENERATED_BODY()

protected:
	/** The user-facing display name of the ItemContainer. */
	UPROPERTY(EditDefaultsOnly, Category = "Item Container|UI")
	FText DisplayName;

	/** The ViewModel to create for the ItemContainer. */
	UPROPERTY(EditDefaultsOnly, Category = "Item Container|UI")
	TSoftClassPtr<UItemContainerViewModel> ItemContainerViewModelClass;

	/** Limits the number of individual item instances that can be in the ItemContainer. */
	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Item Container", SaveGame)
	FQuantityLimit CapacityLimit;

	/** Automatically combine and stack items that are added to the ItemContainer. */
	UPROPERTY(EditDefaultsOnly, Category = "Item Container")
	bool bAutoStack = false;

	/** Tags the ItemContainer has. */
	UPROPERTY(EditDefaultsOnly, Category = "Item Container")
	FGameplayTagContainer OwnedTags;

	/** Additional rules that govern if an item can be added to the ItemContainer. */
	UPROPERTY(EditDefaultsOnly, Category = "Item Container|Rule")
	TArray<TObjectPtr<UItemContainerRule_AddItem>> ItemContainerRules_AddItem;

	/** Additional rules that govern if an item can be directly removed from the ItemContainer. */
	UPROPERTY(EditDefaultsOnly, Category = "Item Container|Rule")
	TArray<TObjectPtr<UItemContainerRule_RemoveItem>> ItemContainerRules_RemoveItem;

	/** Additional rules that govern an item's MaxQuantity and MaxNumberOfStacks. */
	UPROPERTY(EditDefaultsOnly, Category = "Item Container|Rule")
	TArray<TObjectPtr<UItemContainerRule_ItemQuantity>> ItemContainerRules_ItemQuantity;

public:
	UItemContainer();
	virtual void PostInitProperties() override;
	virtual bool IsSupportedForNetworking() const override {return true;}
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif

	/** Called when an item is added to the container. */
	FItemContainerItemInstanceSignature OnItemAddedDelegate;
	/** Called when an item is removed from the container. */
	FItemContainerItemInstanceSignature OnItemRemovedDelegate;
	/** Called when an item's property has changed in the container. */
	FItemContainerItemInstanceSignature OnItemChangedDelegate;

	/** Returns the user facing display name. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	FText GetDisplayName() const {return DisplayName;}

	/** Returns the view model class.*/
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	TSoftClassPtr<UItemContainerViewModel> GetViewModelClass() const {return ItemContainerViewModelClass;}

	/** Returns the ItemContainer's owned gameplay tags. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	const FGameplayTagContainer& GetOwnedTags() const;

	/** Gets the InventoryManager component that owns the ItemContainer. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	UInventoryManagerComponent* GetInventoryManagerComponent() const {return InventoryManagerComponent;}

	//---------------------------------------------------------------------------------------------------------
	// ItemContainer Capacity Functions
	//---------------------------------------------------------------------------------------------------------

	/**
	* Set's the MaxCapacity for the ItemContainer.
	* @param NewCapacity The amount to adjust the MaxCapacity number.
	* @param bOverride If true, NewCount will override the current MaxCapacity. If false, the NewCount will
	* be added to the MaxCapacity.
	* @return Returns the new max capacity.
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Item Container")
	int32 SetMaxCapacity(int32 NewCapacity, bool bOverride = false);

	/** @return The max number of item instances allowed in the ItemContainer. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	int32 GetMaxCapacity() const;

	/** @return Returns the number of ItemInstances in the ItemContainer. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	int32 GetConsumedCapacity() const;

	/**
	 * The remanining number of ItemInstances allowed to be added to the ItemContainer.
	 * @return GetMaxCapacity() - GetConsumedCapacity()
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	int32 GetRemainingCapacity() const;

	/** @return GetConsumedCapacity() >= GetMaxCapacity() */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	bool IsAtMaxCapacity() const;

	//---------------------------------------------------------------------------------------------------------
	// ItemInstance Accessor Functions
	//---------------------------------------------------------------------------------------------------------
	
	/** Returns a const reference of all items in the ItemContainer */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory System|Item Container")
	const TArray<FItemInstance>& GetItems() const;

	/**
	 * @param Guid The unique Guid to search for in this Container.
	 * @return A pointer to the item matching the ItemGuid.
	 */
	FItemInstance* FindItemByGuid(FGuid Guid) const;

	/**
	 * @param Guid The unique Guid to search for.
	 * @return A copy of the item matching the ItemGuid.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container", DisplayName = "Find Item By Guid")
	FItemInstance K2_FindItemByGuid(FGuid Guid) const;

	/**
	 * @return The first item found with the matching ItemDefinition.
	 */
	FItemInstance* FindItemByDefinition(const UItemDefinition* ItemDefinition) const;

	/**
	 * @return The first item found with the matching ItemDefinition.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container", DisplayName = "Find Item By Definition")
	FItemInstance K2_FindItemByDefinition(const UItemDefinition* ItemDefinition) const;

	/**
	 * @return All items in the container by ItemDefinition.
	 */
	TArray<FItemInstance*> FindItemsByDefinition(const UItemDefinition* ItemDefinition) const;

	/**
	 * @return All items in the container by ItemDefinition.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory System|Item Container", DisplayName = "Find Items By Definition")
	TArray<FItemInstance> K2_FindItemsByDefinition(const UItemDefinition* ItemDefinition) const;

	/**
	 * Returns the first item that matches the Item.
	 * See FItem::IsMatching
	 */
	FItemInstance* FindMatchingItem(const TInstancedStruct<FItem>& Item) const;

	/**
	 * Returns the first item that matches the Item.
	 * See FItem::IsMatching
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory System|Item Container", DisplayName = "Find Matching Item")
	FItemInstance K2_FindMatchingItem(const TInstancedStruct<FItem>& Item) const;

	/**
	 * @note See FItem::IsMatching
	 * @param Item The item to check against.
	 * @return Pointers to all matching items.
	 */
	TArray<FItemInstance*> FindMatchingItems(const TInstancedStruct<FItem>& Item) const;

	/**
	 * @note See FItem::IsMatching
	 * @param Item The item to check against.
	 * @return A copy of all items that match the Item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory System|Item Container", DisplayName = "Find Matching Items")
	TArray<FItemInstance> K2_FindMatchingItems(const TInstancedStruct<FItem>& Item) const;

	//---------------------------------------------------------------------------------------------------------
	// ~ItemInstance Accessor Functions
	//---------------------------------------------------------------------------------------------------------

	//---------------------------------------------------------------------------------------------------------
	// Item Query Functions
	//---------------------------------------------------------------------------------------------------------
	
	/**
	 * Checks to see if the item can be added to the ItemContainer.
	 * @param Item The item to check.
	 * @param OutError The error if applicable.
	 * @return True, if the item can be added to the Container.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	virtual bool CanAddItem(UPARAM(ref) const TInstancedStruct<FItem>& Item, FGameplayTag& OutError) const;

	/**
	 * @param Item The item to check.
	 * @return True, if the Item can be removed from the ItemContainer directly without being consumed.
	 */
	virtual bool CanRemoveItem(const TInstancedStruct<FItem>& Item) const;

	/**
	 * @param ItemInstance The item to check if it can be split.
	 * @param Quantity The amount to try and split off from the Item.
	 * @return True, if the item can be split into two stacks in the ItemContainer.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	virtual bool CanSplitItemStack(UPARAM(ref) const FItemInstance& ItemInstance, const int32 Quantity) const;

	/**
	 * Checks to see if the items are matching and how much of the SourceItem can be added to the TargetItem.
	 * @param SourceItemInstance The item you want to take Quantity from to add to TargetItem.
	 * @param TargetItemInstance The item you want to add Quantity to.
	 * @param QuantityToStack The amount of item you want to take from the SourceItem and place into the TargetItem.
	 * @param OutQuantity The quantity allowed to be added to the TargetItem.
	 * @return True, if SourceItemInstance can be stacked into the TargetItemInstance.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	virtual bool CanStackItems(const FItemInstance& TargetItemInstance, const FItemInstance& SourceItemInstance, const int32 QuantityToStack, int32& OutQuantity) const;

	/**
	 * Finds all items by definition and returns the total quantity in the ItemContainer
	 * @param Item The item to check.
	 * @return The total number of item quantity.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	int32 GetTotalItemQuantity(const TInstancedStruct<FItem>& Item) const;
	
	/**
	 * Will check the ItemContainerRules and returns the most restrictive value.
	 * @param Item The item to check.
	 * @return Return the maximum allowed quantity for a single stack of an item.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	virtual int32 GetItemQuantityLimit(const TInstancedStruct<FItem>& Item) const;

	/**
	 * Will check the ItemContainerRules for ItemContainer and InventoryManager and return the most restrictive.
	 * @param Item The Item to check.
	 * @return Returns the maximum number of unique item instances allowed in the ItemContainer.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	virtual int32 GetItemStackQuantityLimit(const TInstancedStruct<FItem>& Item) const;

	/**
	 * Iterates through all Items from the owning ItemManager for items with a matching ItemDefinition.
	 * GetItemStackQuantityLimit - ItemStackCount.
	 * @param Item The Item to check.
	 * @return The number of remaining item stacks that are allowed to be added to the ItemContainer.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	virtual int32 GetRemainingItemStackCapacity(const TInstancedStruct<FItem>& Item) const;

	/**
	 * Checks if the GetRemainingItemStackCapacity is less than or equal to 0.
	 * @param Item The item to check.
	 * @return True, if GetRemainingItemStackCapacity <= 0.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	bool IsItemAtStackQuantityLimit(const TInstancedStruct<FItem>& Item) const;

	//---------------------------------------------------------------------------------------------------------
	// ~Item Query Functions
	//---------------------------------------------------------------------------------------------------------
	
	/**
	 * You must manually call this when an Item stored in this ItemContainer has been modified.
	 */
	void MarkItemDirty(FItemInstance& ItemInstance);

	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Container")
	bool HasAuthority() const {return bOwnerIsNetAuthority;}

protected:

	/**
	 * Is called from the InventoryManagerComponent upon creating this ItemContainer.
	 * This serves as a 'BeginPlay' type function.
	 */
	virtual void Initialize();

	/**
	 * Called from the ItemManager when trying to add an item to the ItemContainer. Should be overriden by subclasses to
	 * determine custom logic. By default, it will add the item without any changes.
	 * @param Item The item to evaluate.
	 * @param AddItemPlan The plan that was passed in to be updated with entries.
	 * @return The collection of items added or modified in the Container.
	 */
	virtual void GetAddItemPlan(const TInstancedStruct<FItem>& Item, FAddItemPlan& AddItemPlan) const;

	/** Called [client + server] when an item is added to the container. */
	virtual void OnItemAdded(const FItemInstance& ItemInstance){}
	/** Called [client + server] when an item is added to the container. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|Item Container", DisplayName = "OnItemAdded")
	void K2_OnItemAdded(const FItemInstance& ItemInstance);

	/** Called [client + server] when an item is removed from the container. */
	virtual void OnItemRemoved(const FItemInstance& ItemInstance){}
	/** Called [client + server] when an item is removed from the container. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|Item Container", DisplayName = "OnItemRemoved")
	void K2_OnItemRemoved(const FItemInstance& ItemInstance);

	/** Called [client + server] when an item has been changed in the container. */
	virtual void OnItemChanged(const FItemInstance& ItemInstance) {}
	/** Called [client + server] when an item has been changed in the container. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|Item Container", DisplayName = "OnItemChanged")
	void K2_OnItemChanged(const FItemInstance& ItemInstance);

private:
	/** All the Items contained in the ItemContainer. */
	UPROPERTY(Replicated)
	FItemInstanceContainer ItemInstanceContainer;

	/** The owner of this ItemContainer. */
	UPROPERTY(Transient)
	TObjectPtr<UInventoryManagerComponent> InventoryManagerComponent;
	UPROPERTY()
	bool bOwnerIsNetAuthority = false;

	friend UInventoryManagerComponent;
	friend struct FItemContainerInstance;
	friend struct FItemContainerInstanceContainer;
	friend struct FItemInstance;
	friend struct FItemInstanceContainer;

	void Internal_OnItemAdded(const FItemInstance& ItemInstance);
	void Internal_OnItemRemoved(const FItemInstance& ItemInstance);
	void Internal_OnItemChanged(const FItemInstance& ItemInstance);
};
