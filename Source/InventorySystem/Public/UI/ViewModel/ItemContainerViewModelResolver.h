// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "View/MVVMViewModelContextResolver.h"
#include "ItemContainerViewModelResolver.generated.h"

class UItemContainerViewModel;
class UItemContainerProvider;
class UItemContainer;

/**
 * Base class for a view model resolver for InventoryItems.
 */
UCLASS(Abstract)
class INVENTORYSYSTEM_API UItemContainerViewModelResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportViewModelClass(const UClass* Class) const override;

	/** Get or creates a view model for the relevant InventoryContainer. */
	virtual UItemContainerViewModel* GetItemContainerViewModel(const UUserWidget* UserWidget, const UMVVMView* View) const;

	/** Returns the relevant ItemContainer. */
	virtual UItemContainer* GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const;
};

/**
 * Resolves an ItemContainer view model using a provider and Tag.
 * Implement custom provider classes to retrieve containers from different actors in the world.
 */
UCLASS()
class INVENTORYSYSTEM_API UItemContainerViewModelResolver_ItemContainerTag : public UItemContainerViewModelResolver
{
	GENERATED_BODY()

public:
	UItemContainerViewModelResolver_ItemContainerTag();

	/** The InventoryContainer provider to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewmodel")
	TSubclassOf<UItemContainerProvider> Provider;

	/** The ItemContainer tag to search for. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewmodel", meta = (Categories = "InventoryContainer"))
	FGameplayTag ItemContainerTag;

	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
	virtual UItemContainer* GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const override;
};