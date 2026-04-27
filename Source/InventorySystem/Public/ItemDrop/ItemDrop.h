// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "ItemContainer/ItemContainer.h"
#include "GameFramework/Actor.h"
#include "ItemDrop.generated.h"

struct FGameplayTag;
struct FItemInstance;
class UInventoryManagerComponent;
class UItemContainer;
class AItemDropManager;


/**
 * An actor that represents a single instance of an Item. This can be taken by another actor with an InventoryManagerComponent.
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class INVENTORYSYSTEM_API AItemDrop : public AActor
{
	GENERATED_BODY()

public:
	AItemDrop();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * The InventoryManager will try and take the item from this ItemDropActor.
	 * @param InventoryManagerComponent The InventoryManager trying to take the item.
	 * @param ItemContainer The Container to add the item to.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Item Drop")
	void TakeItem(UInventoryManagerComponent* InventoryManagerComponent, UItemContainer* ItemContainer);

	/**
	 * Decides if the passed in InventoryManager can attempt to take this ItemDrop.
	 * @param InventoryManagerComponent The InventoryManagerComponent to check if it can take the item.
	 * @return True if the InventoryManager can take the item.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Drop")
	virtual bool CanTakeItem(UInventoryManagerComponent* InventoryManagerComponent) const;

	/**
	 * @return A copy of the ItemInstance held by this item drop.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Drop")
	FItemInstance GetItemInstance() const;

protected:
	/**
	 * Initializes this actor with an Item and optional context data.
	 * @param Handle The item to assign this ItemDrop.
	 * @param Context Custom user data that can be passed in and processed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Item Drop")
	virtual void InitializeItemDrop(FItemInstanceHandle Handle, UObject* Context);

	/**
	 * Called on the server when this actor has been initialized with an item.
	 * @param Handle The item this actor was initialized with. The Item is guaranteed to be valid.
	 * @param Context Custom user data that can be passed in and processed.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "InitializeItemDrop")
	void K2_InitializeItemDrop(FItemInstanceHandle Handle, UObject* Context);

	/**
	 * Decides if the passed in InventoryManager can attempt to take this ItemDrop.
	 * @param InventoryManager The InventoryManager to check if it can take the item.
	 * @return True if the InventoryManager can take the item.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|Item Drop", DisplayName = "CanTakeItem")
	bool K2_CanTakeItem(UInventoryManagerComponent* InventoryManager) const;

private:
	/** The item held by this actor. */
	UPROPERTY(Replicated)
	FItemInstanceHandle ItemInstanceHandle;

	/** Cached reference to the ItemDropManager that owns this. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<AItemDropManager> ItemDropManager;

	uint8 bHasCanTakeItem : 1;

	friend AItemDropManager;
};
