// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "ItemDropTypes.h"
#include "GameFramework/Info.h"
#include "StructUtils/InstancedStruct.h"
#include "ItemDropManager.generated.h"

struct FItemInstance;
struct FItem;
struct FItemDropParams;
class AItemDrop;
class UInventoryManagerComponent;
class UItemContainer;

/**
 * Manages the replication for ItemDrops and their items.
 */
UCLASS(BlueprintType, Blueprintable)
class INVENTORYSYSTEM_API AItemDropManager : public AInfo
{
	GENERATED_BODY()

	friend AItemDrop;
	
	UPROPERTY()
	TObjectPtr<UInventoryManagerComponent> InventoryManagerComponent;
	
public:
	AItemDropManager();
	virtual void BeginPlay() override;

	/**
	 * Takes the passed in Item and tries to represent it in the world.
	 * @param Item The item to represent in the world.
	 * @param Quantity The amount of the item.
	 * @param Params Defines the parameters for spawning the ItemDropActor.
	 * @return The newly created ItemDropActor.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Item Drop Manager")
	AItemDrop* TryCreateItemDrop(UPARAM(ref) const TInstancedStruct<FItem>& Item, const int32 Quantity, const FItemDropParams& Params);

	/**
	 * Takes the ItemInstance and consumes the specified QuantityToDrop.
	 * @param ItemInstance The ItemInstance to drop in full or partial.
	 * @param Params Defines the parameters for spawning the ItemDropActor.
	 * @param QuantityToDrop The amount to take from the ItemInstance.
	 * @return The newly created ItemDropActor.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Item Drop Manager")
	AItemDrop* TryCreateItemDropFromItemInstance(
		UPARAM(ref) const FItemInstance& ItemInstance,
		const FItemDropParams& Params,
		const int32 QuantityToDrop = 1);

protected:

	/** The maximum number of ItemDropActors allowed to be spawned in the world. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxItemDrops = 100;

	void RemoveItemDrop(AItemDrop* ItemDrop);

private:
	UPROPERTY()
	TArray<TObjectPtr<AItemDrop>> ItemDrops;

	// Cached off value of the InventoryContainer
	UPROPERTY()
	TObjectPtr<UItemContainer> InventoryContainer_Drop;

	/** Creates the ItemDrop and adds it to the array. */
	AItemDrop* Internal_CreateItemDrop(const FGuid ItemGuid, const FItemDropParams& Params);

	/** If at MaxItemDrops, makes enough space for one new item drop. */
	void ClearItemDrops();

	UFUNCTION()
	void OnItemRemoved(UInventoryManagerComponent* InventoryManager, const FItemInstance& ItemInstance);

	void Internal_RemoveItemDrop(AItemDrop* ItemDrop);
};
