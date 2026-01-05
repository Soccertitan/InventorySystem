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
	
	/** Updates the ItemContainer for this ViewModel. Triggers OnItemContainerSet if a new one is set. */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|View Model")
	void SetItemContainer(UItemContainer* InItemContainer);

	UFUNCTION(BlueprintPure, Category = "Inventory System|View Model")
	UItemContainer* GetItemContainer() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	FText GetItemContainerName() const {return ItemContainerName;}

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	int32 GetConsumedCapacity() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	int32 GetMaxCapacity() const;

	/** Called only once when the ItemInstanceViewModels are first created in SetItemContainer. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	TArray<UItemInstanceViewModel*> GetItemInstanceViewModels() const;

	/** Called whenever an ItemInstanceViewModel is added to the ItemInstanceViewModels after initial setup. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	UItemInstanceViewModel* GetAddedItemInstanceViewModel() const {return ItemInstanceViewModelBuffer;}

	/** Called whenever an ItemInstanceViewModel is removed to the ItemInstanceViewModels. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	UItemInstanceViewModel* GetRemovedItemInstanceViewModel() const {return ItemInstanceViewModelBuffer;}

	/** Called whenever an ItemInstanceViewModel is changed in the ItemInstanceViewModels. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	UItemInstanceViewModel* GetChangedItemInstanceViewModel() const {return ItemInstanceViewModelBuffer;}

protected:
	/** The ItemInstanceViewModelClass to create. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, NoClear, Category = "Item Container View Model")
	TSubclassOf<UItemInstanceViewModel> ItemInstanceViewModelClass;

	/** Called when a valid ItemContainer is set. */
	virtual void OnItemContainerSet() {}

	/** Called whenever an item is added to the ItemContainer. */
	virtual void OnItemAdded(const FItemInstance& ItemInstance, UItemInstanceViewModel* ItemInstanceViewModel) {}
	/** Called whenever an item is removed from the ItemContainer. */
	virtual void OnItemRemoved(const FItemInstance& ItemInstance, UItemInstanceViewModel* ItemInstanceViewModel) {}
	/** Called whenever an item is changed in the ItemContainer. */
	virtual void OnItemChanged(const FItemInstance& ItemInstance, UItemInstanceViewModel* ItemInstanceViewModel) {}

	/**
	 * Creates an ItemInstanceViewModel using the ItemInstanceViewModelClass.
	 */
	UItemInstanceViewModel* CreateItemInstanceViewModel(const FItemInstance& ItemInstance);

	static bool DoesItemHaveUIFragment(const TInstancedStruct<FItem>& Item);

private:
	UPROPERTY()
	TWeakObjectPtr<UItemContainer> WeakItemContainer;

	/** The created ItemInstanceViewModels. */
	UPROPERTY()
	TArray<TObjectPtr<UItemInstanceViewModel>> ItemInstanceViewModels;

	/** Temporarily updated as items are added, removed, and changed. */
	UPROPERTY()
	TObjectPtr<UItemInstanceViewModel> ItemInstanceViewModelBuffer;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = true))
	FText ItemContainerName;

	void Internal_OnItemAdded(UItemContainer* InContainer, const FItemInstance& ItemInstance);
	void Internal_OnItemRemoved(UItemContainer* InContainer, const FItemInstance& ItemInstance);
	void Internal_OnItemChanged(UItemContainer* InContainer, const FItemInstance& ItemInstance);
};
