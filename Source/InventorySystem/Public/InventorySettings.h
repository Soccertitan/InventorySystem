// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "InventorySettings.generated.h"

class UItemContainer;

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig)
class INVENTORYSYSTEM_API UInventorySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UInventorySettings();
	virtual FName GetCategoryName() const override;

	/** The default tag to use for a root InventoryContainer. Need to ensure the tag has a valid map in the settings. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta = (Categories = "ItemContainer"))
	FGameplayTag DefaultItemContainerTag;

	/** The default ItemContainer to use. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UItemContainer> DefaultItemContainerClass;

	UFUNCTION(BlueprintPure, Category = "Inventory System|Settings")
	static FGameplayTag GetDefaultItemContainerTag();
	UFUNCTION(BlueprintPure, Category = "Inventory System|Settings")
	static TSubclassOf<UItemContainer> GetDefaultItemContainerClass();
};
