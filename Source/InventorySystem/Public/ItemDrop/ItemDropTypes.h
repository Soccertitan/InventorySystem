// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "ItemDropTypes.generated.h"


class AItemDrop;

/**
 * Params for the ItemDropManager on how to create a new ItemDrop actor.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemDropParams
{
	GENERATED_BODY()

	/** The ItemDrop actor class to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AItemDrop> ItemDropClass;

	/** The spawn location for the ItemDrop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SpawnLocation = FVector();

	/** The context for dropping the item. This will be passed along to the ItemDrop. */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> Context;

	bool IsValid() const;
};
