// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Item/ItemDefinition.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"
#include "InventoryBlueprintFunctionLibrary.generated.h"

struct FItemFragment;
class UItemDefinition;
class UInventoryManagerComponent;

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get the ItemManagerComponent if the actor implements the InventorySystemInterface. If it does not, fallbacks to
	 * checking the actor's components.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System", meta = (DefaultToSelf = "Actor"))
	static UInventoryManagerComponent* GetInventoryManagerComponent(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Inventory System|Item")
	static bool IsItemInstanceValid(UPARAM(ref) const FItemInstance& ItemInstance);

	/**
	 * Synchronously loads the ItemDefinition from the Item. Does not flush any current AsyncLoads.
	 * @param Item The item to retrieve the ItemDefinition from.
	 * @return The loaded ItemDef or nullptr.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Item")
	static const UItemDefinition* GetItemDefinition(UPARAM(ref) const TInstancedStruct<FItem>& Item);

	/**
	 * Finds the first fragment.
	 * @param Item The item to retrieve the Fragment from.
	 * @param FragmentType The type of item fragment to search for.
	 * @return A copy of the Fragment.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|Item", DisplayName = "Find Item Shard")
	static TInstancedStruct<FItemShard> K2_FindItemShard(
		UPARAM(ref) const TInstancedStruct<FItem>& Item,
		UPARAM(meta = (MetaStruct="/Script/InventorySystem.ItemShard")) const UScriptStruct* FragmentType);

	/**
	 * Iterates through an ItemDefinition's Fragments and finds the one that is a child of FragmentType.
	 * @param ItemDefinition The ItemDefinition to get the ItemDefinitionFragment from.
	 * @param FragmentType The type of fragment to search for.
	 * @return A copy of ItemFragment of the FragmentType.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|Item", DisplayName = "Find Item Fragment")
	static TInstancedStruct<FItemFragment> K2_FindItemFragment(
		const UItemDefinition* ItemDefinition,
		UPARAM(meta = (MetaStruct="/Script/InventorySystem.ItemFragment")) const UScriptStruct* FragmentType);
};
