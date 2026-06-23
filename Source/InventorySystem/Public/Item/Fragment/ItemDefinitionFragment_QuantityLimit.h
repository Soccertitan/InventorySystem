// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemDefinition.h"
#include "ItemDefinitionFragment_QuantityLimit.generated.h"

/**
 * Describes the limitation for stacking and adding items.
 */
USTRUCT(BlueprintType)
struct FItemDefinitionFragment_QuantityLimit : public FItemDefinitionFragment
{
	GENERATED_BODY()

	/** The maximum quantity allowed in a single ItemInstance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FQuantityLimit Item;

	/** The maximum number of unique ItemInstances allowed in a single ItemContainer. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FQuantityLimit ItemContainer;
};
