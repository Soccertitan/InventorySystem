// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InventoryUISubsystem.generated.h"

struct FGameplayTag;
struct FItemContainerViewContext;
class UItemContainerViewModel;
class UItemContainerProvider;
class UItemContainer;

/**
 * Subsystem for working with game items UI.
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryUISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Get a view model for an InventoryContainer, reusing an existing one if it already exists. */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|UISubsystem")
	UItemContainerViewModel* CreateItemContainerViewModel(UItemContainer* ItemContainer);

	/** Get a view model for an InventoryContainer, reusing an existing one if it already exists. */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|UISubsystem")
	UItemContainerViewModel* CreateItemContainerViewModelForActor(AActor* Actor,
		UPARAM (meta = (Categories = "ItemContainer")) FGameplayTag ItemContainerTag);

	/** Retrieve an InventoryContainer from a provider class, given view context. */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|UISubsystem")
	UItemContainer* GetItemContainerFromProvider(TSubclassOf<UItemContainerProvider> Provider,
		UPARAM (meta = (Categories = "ItemContainer")) FGameplayTag ItemContainerTag, const FItemContainerViewContext& Context);

private:
	/** All container view models that have been created. */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UItemContainerViewModel>> InventoryContainerViewModels;

	UItemContainerViewModel* Internal_CreateContainerViewModel(UItemContainer* Container);
};
