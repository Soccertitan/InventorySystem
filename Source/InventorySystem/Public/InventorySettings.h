// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "InventorySettings.generated.h"

class UItemInstanceViewModel;
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
	FGameplayTag ItemContainerTag;

	/** The default ItemContainer to use. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UItemContainer> ItemContainerClass;
	
	/** The default ItemInstanceViewModle to use. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UItemInstanceViewModel> ItemInstanceViewModelClass;

	UFUNCTION(BlueprintPure, Category = "Inventory System|Settings")
	static FGameplayTag GetItemContainerTag();
	UFUNCTION(BlueprintPure, Category = "Inventory System|Settings")
	static TSubclassOf<UItemContainer> GetItemContainerClass();
	UFUNCTION(BlueprintPure, Category = "Inventory System|Settings")
	static TSubclassOf<UItemInstanceViewModel> GetItemInstanceViewModelClass();
};
