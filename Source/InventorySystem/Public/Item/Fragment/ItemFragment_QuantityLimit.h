// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemDefinition.h"
#include "ItemFragment_QuantityLimit.generated.h"

/**
 * Describes the limitation for stacking and adding items.
 */
USTRUCT(BlueprintType)
struct FItemFragment_QuantityLimit : public FItemFragment
{
	GENERATED_BODY()

	/** The maximum quantity allowed in a single ItemInstance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FQuantityLimit Item;

	/** The maximum number of unique ItemInstances allowed in a single ItemContainer. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FQuantityLimit ItemContainer;

	/** The maximum number of unique ItemInstances allowed across all ItemContainers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FQuantityLimit InventoryManager;
};
