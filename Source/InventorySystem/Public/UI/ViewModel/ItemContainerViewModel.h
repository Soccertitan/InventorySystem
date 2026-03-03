// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "StructUtils/InstancedStruct.h"
#include "ItemContainerViewModel.generated.h"

struct FItem;
class UInventoryManagerComponent;
class UItemInstanceViewModel;
class UItemContainer;
struct FItemInstance;

/**
 * A generic implementation of the ItemContainer ViewModel designed to be subclassed. It will add and remove items to the
 * ItemInstanceViewModels array as needed. And supply changes to the ItemInstanceViewModels as needed.
 */
UCLASS()
class INVENTORYSYSTEM_API UItemContainerViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UItemContainerViewModel();

	UFUNCTION(BlueprintPure, Category = "Inventory System|View Model")
	UItemContainer* GetItemContainer() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	FText GetItemContainerName() const {return ItemContainerName;}

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	int32 GetConsumedCapacity() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	int32 GetMaxCapacity() const;

	/** Called only once when the ItemInstanceViewModels are first created in SetItemContainer. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model", DisplayName = GetItemInstanceViewModels)
	TArray<UItemInstanceViewModel*> K2_GetItemInstanceViewModels() const;
	
	/** Returns a const reference to the array of ItemInstanceViewModels. */
	const TArray<UItemInstanceViewModel*>& GetItemInstanceViewModels() const;

	/** Called whenever an ItemInstanceViewModel is added to the ItemInstanceViewModels. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	UItemInstanceViewModel* GetAddedItemInstanceViewModel() const {return ItemInstanceViewModelBuffer;}

	/** Called whenever an ItemInstanceViewModel is removed to the ItemInstanceViewModels. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	UItemInstanceViewModel* GetRemovedItemInstanceViewModel() const {return ItemInstanceViewModelBuffer;}

	/** Called whenever an ItemInstanceViewModel is changed in the ItemInstanceViewModels. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	UItemInstanceViewModel* GetChangedItemInstanceViewModel() const {return ItemInstanceViewModelBuffer;}

protected:
	/** Updates the ItemContainer for this ViewModel. Triggers OnItemContainerSet if a new one is set. */
	void SetItemContainer(UItemContainer* InItemContainer);
	
	/** Called when a valid ItemContainer is set. */
	virtual void OnItemContainerSet() {}

	/** Called whenever an item is added to the ItemContainer. */
	virtual void OnItemAdded(UItemInstanceViewModel* ItemInstanceViewModel) {}
	/** Called whenever an item is removed from the ItemContainer. */
	virtual void OnItemRemoved(UItemInstanceViewModel* ItemInstanceViewModel) {}
	/** Called whenever an item is changed in the ItemContainer. */
	virtual void OnItemChanged(UItemInstanceViewModel* ItemInstanceViewModel) {}

private:
	UPROPERTY()
	TObjectPtr<UItemContainer> ItemContainer;

	/** The created ItemInstanceViewModels. */
	UPROPERTY()
	TArray<TObjectPtr<UItemInstanceViewModel>> ItemInstanceViewModels;

	/** Temporarily updated as items are added, removed, and changed. */
	UPROPERTY()
	TObjectPtr<UItemInstanceViewModel> ItemInstanceViewModelBuffer;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = true))
	FText ItemContainerName;
	
	/** Loads the ItemDefinition for all Items to get the ItemInstanceViewModelClass. */
	void LoadItemDefinitions(const TArray<FItemInstance>& ItemInstances);
	void ItemDefinitionsLoaded(TArray<FItemInstance> ItemInstances);

	void Internal_OnItemAdded(const FItemInstance& ItemInstance);
	void Internal_OnItemRemoved(const FItemInstance& ItemInstance);
	void Internal_OnItemChanged(const FItemInstance& ItemInstance);
	
	friend class UInventoryUISubsystem;
};
