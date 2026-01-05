// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "InventorySaveDataTypes.generated.h"

struct FGameplayTag;
struct FItemContainerInstance;
struct FItemInstance;
class UItemDefinition;
class UItemContainer;

/** Contains the save data for an item. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemSaveData
{
	GENERATED_BODY()

	FItemSaveData(){}

	FItemSaveData(FItemInstance& ItemInstance);

	/** The unique identifier for the item. */
	UPROPERTY(BlueprintReadOnly)
	FGuid Guid;

	/** The item definition to check if it's valid before restoring the item. */
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UItemDefinition> ItemDefinition;
	
	/** The quantity of the item. */
	UPROPERTY(BlueprintReadOnly)
	int32 Quantity = 1;

	/** The item's serialized SaveGame properties. */
	UPROPERTY()
	TArray<uint8> ByteData;
};

/** Contains the save data of the ItemContainer. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemContainerSaveData
{
	GENERATED_BODY()

	FItemContainerSaveData(){}

	FItemContainerSaveData(const FItemContainerInstance& ItemContainerInstance);

	/** The InventoryContainers lookup tag. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ContainerTag = FGameplayTag();

	/** The class of ItemContainer to spawn. */
	UPROPERTY(BlueprintReadOnly)
	TSoftClassPtr<UItemContainer> ContainerClass;

	/** The ItemContainer's serialized SaveGame properties. */
	UPROPERTY()
	TArray<uint8> ByteData;

	/** The item save data. */
	UPROPERTY(BlueprintReadOnly)
	TArray<FItemSaveData> InventoryItemsSaveData;
};
