// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "InventoryFastTypes.h"
#include "InventorySaveDataTypes.h"
#include "InventoryTypes.h"
#include "Components/ActorComponent.h"
#include "ItemContainer/ItemContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "InventoryManagerComponent.generated.h"


class UItemDefinition;
class UItemContainer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryManagerItemSignature, UInventoryManagerComponent*, InventoryManagerComponent, const FItemInstance&, ItemInstance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryManagerContainerSignature, UInventoryManagerComponent*, InventoryManagerComponent, UItemContainer*, Container);

/**
 * Manages ItemContainers and their ItemInstances.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEM_API UInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryManagerComponent();
	virtual void BeginPlay() override;
	virtual void PreNetReceive() override;
	virtual void OnRegister() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** The startup ItemContainers. Mapped to a GameplayTag for the ItemContainerId. */
	UPROPERTY(EditAnywhere, Category = "InventoryManager", meta=(ForceInlineRow, Categories = "ItemContainer"))
	TMap<FGameplayTag, FStartupItems> StartupItems;

	/** Called [Client + Server] when a new item has been added to a container. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnItemAdded")
	FInventoryManagerItemSignature OnItemAddedDelegate;
	
	/** Called [Client + Server] when an item has been removed from a container. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnItemRemoved")
	FInventoryManagerItemSignature OnItemRemovedDelegate;

	/** Called [Client + Server] when an Item's properties have changed in a container. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnItemChanged")
	FInventoryManagerItemSignature OnItemChangedDelegate;

	/** Called [Client + Server] when a ItemContainer has been added to the list. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnContainerAdded")
	FInventoryManagerContainerSignature OnContainerAddedDelegate;

	/** Called [Client + Server] when a ItemContainer has been added to the list. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnContainerRemoved")
	FInventoryManagerContainerSignature OnContainerRemovedDelegate;

	/**
	 * @param ItemContainerTag The tag to search with.
	 * @return The ItemContainer with the Tag.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Inventory Manager")
	UItemContainer* FindItemContainerByTag(UPARAM(meta = (Categories = "ItemContainer")) FGameplayTag ItemContainerTag) const;

	/**
	 * @param OwnedTags The owned tags a Container has.
	 * @return All ItemContainers that have all the OwnedTags.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory System|Inventory Manager")
	TArray<UItemContainer*> FindItemContainersByOwnedTags(FGameplayTagContainer OwnedTags);

	/** Gets all Containers this ItemManager has. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Inventory Manager")
	const TArray<FItemContainerInstance>& GetItemContainers() const;

	/** Returns a copy of all Items in the ItemManager. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory System|Inventory Manager")
	TArray<FItemInstance> GetItems() const;

	/**
	 * @param ItemGuid The item to search for.
	 * @return A pointer to the found item.
	 */
	FItemInstance* FindItemByGuid(FGuid ItemGuid) const;

	/**
	 * @param Guid The item to search for.
	 * @return A copy of the found item.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Inventory Manager", DisplayName = "Find Item By Guid")
	FItemInstance K2_FindItemByGuid(FGuid Guid) const;

	/**
	 * @param ItemDefinition The ItemDefinition to check.
	 * @return A pointer of all items with matching item definitions.
	 */
	TArray<FItemInstance*> FindItemsByDefinition(const UItemDefinition* ItemDefinition) const;

	/**
	 * @param ItemDefinition The ItemDef to check.
	 * @return A copy of all items with matching item definitions.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory System|Inventory Manager", DisplayName = "Find Items By Definition")
	TArray<FItemInstance> K2_FindItemsByDefinition(const UItemDefinition* ItemDefinition) const;

	/**
	 * Creates a new item instance and initializes it with the ItemDefinition.
	 * @param ItemDefinition The ItemDefinition to associate with the item.
	 * @param Quantity The number of items in the stack with a minimum set to 1.
	 * @return The newly created item.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|Inventory Manager")
	static TInstancedStruct<FItem> CreateItem(const UItemDefinition* ItemDefinition, const int32 Quantity = 1);

	/**
	 * Duplicates the item and assigns a new quantity to it.
	 * @param Item The item to duplicate.
	 * @param Quantity The number of items in the stack with a minimum set to 1.
	 * @return An exact copy of the item with a new quantity.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|Inventory Manager")
	static TInstancedStruct<FItem> DuplicateItem(UPARAM(ref) const TInstancedStruct<FItem>& Item, const int32 Quantity = 1);

	/**
	 * Tries to add a new item that will be managed by this ItemManager.
	 * @param Item The item to add.
	 * @param ItemContainer The container to add the item to.
	 * @return The actual amount of the item that was added and any errors if the item could not be added in full.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manager")
	FAddItemPlanResult TryAddItem(UPARAM(ref) const TInstancedStruct<FItem>& Item, UItemContainer* ItemContainer);

	/**
	 * Tries to add a new item that will be managed by this ItemManager.
	 * @param Item The item to add.
	 * @param ItemContainerTag The container to add the item to.
	 * @return The actual amount of the item that was added and any errors if the item could not be added in full.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manager")
	FAddItemPlanResult TryAddItemByGuid(UPARAM(ref) const TInstancedStruct<FItem>& Item,
		UPARAM(meta = (Categories = "ItemContainer")) const FGameplayTag ItemContainerTag);

	/**
	 * Directly removes the Item from the ItemManager.
	 * @param ItemInstance The ItemInstance to remove.
	 * @return A copy of the Item that was removed.
	 */
	TInstancedStruct<FItem> TryRemoveItem(FItemInstance* ItemInstance);

	/**
	 * Directly removes the Item from the ItemManager.
	 * @param ItemGuid The Item's identifier to remove.
	 * @return A copy of the Item that was removed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manager")
	TInstancedStruct<FItem> TryRemoveItemByGuid(const FGuid ItemGuid);

	/**
	 * Consumes the specified quantity of the item. If the quantity reaches 0, the item is removed from the ItemManager.
	 * @param ItemInstance The item to consume quantity from.
	 * @param QuantityToConsume The amount to subtract from the item. Must be >= 0.
	 * @return A copy of the Item that was consumed with the quantity set to the actual amount consumed.
	 */
	TInstancedStruct<FItem> ConsumeItem(FItemInstance* ItemInstance, const int32 QuantityToConsume);

	/**
	 * Consumes the specified quantity of the item. If the quantity reaches 0, the item is removed from the ItemManager.
	 * @param ItemGuid The item to consume quantity from.
	 * @param QuantityToConsume The amount to subtract from the item. Must be >= 0.
	 * @return A copy of the Item that was consumed with the quantity set to the actual amount consumed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manager")
	TInstancedStruct<FItem> ConsumeItemByGuid(const FGuid ItemGuid, const int32 QuantityToConsume);

	/**
	 * Gets all items with the matching ItemDef. Then subtracts quantity from them until the amount subtracted has reached
	 * 0. Then, if an item's quantity is 0, removes the item from the ItemContainer.
	 * @param ItemDefinition The ItemDef to look for amongst items.
	 * @param QuantityToConsume The amount to subtract from the items.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manager")
	TArray<TInstancedStruct<FItem>> ConsumeItemsByDefinition(const UItemDefinition* ItemDefinition, int32 QuantityToConsume);

	/**
	 * Gets all items with the matching ItemDef in the ItemContainer. Then subtracts quantity from them until the amount subtracted has reached
	 * 0. Then, if an item's quantity is 0, removes the item from the ItemContainer.
	 * @param ItemDefinition The ItemDef to look for amongst items.
	 * @param QuantityToConsume The amount to subtract from the items.
	 * @param ItemContainer The items will only be consumed from the specified ItemContainer.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manager")
	TArray<TInstancedStruct<FItem>> ConsumeItemsByDefinitionInContainer(
		const UItemDefinition* ItemDefinition,
		int32 QuantityToConsume,
		UItemContainer* ItemContainer);

	/**
	 * Tries to move an ItemInstance managed by the ItemManagerComponent into a different ItemContainer. Either in full or partial.
	 * @param ItemInstance The ItemInstance to move.
	 * @param Container The Container to move the ItemInstance into.
	 * @param QuantityToMove The amount from the Item to move into the Container.
	 */
	void TryMoveItem(FItemInstance* ItemInstance, UItemContainer* ItemContainer, int32 QuantityToMove);

	/**
	 * Tries to move an ItemInstance managed by the ItemManagerComponent into a different ItemContainer. Either in full or partial.
	 * @param ItemGuid The ItemInstance to move.
	 * @param ItemContainerTag The Container to move the ItemInstance into.
	 * @param QuantityToMove The amount from the Item to move into the Container.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manager", DisplayName = "TryMoveItem")
	void K2_TryMoveItem(FGuid ItemGuid,
		UPARAM(meta = (Categories = "ItemContainer")) const FGameplayTag ItemContainerTag, int32 QuantityToMove);

	/**
	 * Tries to split the item stack in the existing container.
	 * @param ItemInstance The Item to try and split.
	 * @param Quantity The amount to split off from the original item into to the new item.
	 */
	void SplitItemStack(FItemInstance* ItemInstance, int32 Quantity);

	/**
	 * Tries to split the item stack in the existing container.
	 * @param ItemGuid The Item to try and split.
	 * @param Quantity The amount to split off from the original item into to the new item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manager", DisplayName = "SplitItemStack")
	void K2_SplitItemStack(const FGuid ItemGuid, const int32 Quantity);

	/**
	 * Tries to take quantity from the SourceItem and give it to the TargetItem. To stack items within the same ItemContainer.
	 * @param TargetItemInstance The Item you want to add Quantity to.
	 * @param SourceItemInstance The Item you want to take Quantity from.
	 * @param Quantity The amount from the SourceItem to add to the TargetItem.
	 */
	void StackItems(FItemInstance* TargetItemInstance, FItemInstance* SourceItemInstance, const int32 Quantity);

	/**
	 * Tries to take quantity from the SourceItem and give it to the TargetItem. To stack items within the same ItemContainer.
	 * @param TargetItemGuid The Item you want to add Quantity to.
	 * @param SourceItemGuid The Item you want to take Quantity from.
	 * @param Quantity The amount from the SourceItem to add to the TargetItem.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manager", DisplayName = "StackItems")
	void K2_StackItems(FGuid TargetItemGuid, FGuid SourceItemGuid, const int32 Quantity);

	/**
	 * Creates a new ItemContainer and initializes it.
	 * @param ItemContainerTag The tag to set the new container with. If an existing Tag exists, the ItemContainer will
	 * not be created.
	 * @param ItemContainerClass The Container class to create.
	 * @return The newly created ItemContainer or a nullptr if creating a new one failed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manager")
	UItemContainer* CreateItemContainer(
		UPARAM(meta = (Categories = "ItemContainer")) FGameplayTag ItemContainerTag,
		TSubclassOf<UItemContainer> ItemContainerClass);

	/**
	 * Remove all items from the ItemContainer and then deletes the ItemContainer.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manager")
	void RemoveItemContainer(UItemContainer* ItemContainer);
	
	/**
	 * Collects all the unique item instances and Containers. Saves the data in a struct and serializes that data.
	 * @return The current SaveData for the ItemManager.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Inventory Manager")
	TArray<FItemContainerSaveData> GetSaveData() const;

	/**
	 * Sets the ItemManager to the SavedData's state.
	 * @param SaveData The save data.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manager")
	void LoadSavedData(UPARAM(ref) const TArray<FItemContainerSaveData>& SaveData);

	/* Returns true if this Component's Owner Actor has authority. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Inventory Manager")
	bool HasAuthority() const;

	/**
	 * You must manually call this when an Item stored in this ItemManager has been modified.
	 */
	void MarkItemDirty(FItemInstance& ItemInstance);

protected:

	virtual void OnContainerAdded(const FItemContainerInstance& ContainerInstance);
	virtual void OnContainerRemoved(const FItemContainerInstance& ContainerInstance);

	virtual void OnItemAdded(const FItemInstance& ItemInstance);
	virtual void OnItemRemoved(const FItemInstance& ItemInstance);
	virtual void OnItemChanged(const FItemInstance& ItemInstance);

private:
	/** Cached value of whether our owner is a simulated Actor. */
	UPROPERTY()
	bool bCachedIsNetSimulated = false;
	void CacheIsNetSimulated();

	/** All the Containers managed by the ItemManager component. */
	UPROPERTY(Replicated)
	FItemContainerInstanceContainer InventoryContainerInstanceContainer;

	friend UItemContainer;
	friend struct FItemInstance;
	friend struct FItemInstanceContainer;
	friend struct FItemContainerInstance;
	friend struct FItemContainerInstanceContainer;

	void InitializeStartupItems();

	/**
	 * Adds new items to be managed by this ItemManager.
	 * @param ItemContainer The Container to add the items to.
	 * @param AddItemPlan The AddItemCollection to execute.
	 * @return The Guids of each ItemInstance added/modified in the container. 
	 */
	TArray<FGuid> Internal_ExecuteAddItemPlan(UItemContainer* ItemContainer, const FAddItemPlan& AddItemPlan);

	/**
	 * Moves an existing ItemInstance from one container to the new one following the AddItemPlan rule.
	 * @param ItemInstance The item to move.
	 * @param ItemContainer The Container to move items to.
	 * @param AddItemPlan The plan on how to add items to the Container.
	 */
	void Internal_ExecuteAddItemPlan_Move(FItemInstance* ItemInstance, UItemContainer* ItemContainer, const FAddItemPlan& AddItemPlan);

	/** Adds an Item to the ItemList. */
	void Internal_AddItem(const FGuid ItemGuid, const TInstancedStruct<FItem>& Item, UItemContainer* ItemContainer);

	/** Removes an Item from the ItemList. */
	void Internal_RemoveItem(FGuid ItemGuid, UItemContainer* ItemContainer);
};
