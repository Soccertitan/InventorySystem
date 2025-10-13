// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "StructUtils/InstancedStruct.h"
#include "ItemContainerViewModel.generated.h"

class UInventoryManagerComponent;
class UItemViewModel;
class UItemContainer;
struct FItemInstance;
struct FItem;

/**
 * A generic implementation of the ItemContainer ViewModel designed to be subclassed. It will add and remove items to the
 * ItemViewModels array as needed. And supply changes to the ItemViewModels as needed.
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

	/** Called only once when the ItemViewModels are first created in SetItemContainer. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	TArray<UItemViewModel*> GetItemViewModels() const;

	/** Called whenever an ItemViewModel is added to the ItemViewModels after initial setup. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	UItemViewModel* GetAddedItemViewModel() const {return ItemViewModelBuffer;}

	/** Called whenever an ItemViewModel is removed to the ItemViewModels. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	UItemViewModel* GetRemovedItemViewModel() const {return ItemViewModelBuffer;}

	/** Called whenever an ItemViewModel is changed in the ItemViewModels. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Inventory System|View Model")
	UItemViewModel* GetChangedItemViewModel() const {return ItemViewModelBuffer;}

protected:
	/** The ItemViewModelClass to create for an item if the Item's ItemViewModelClass is null. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, NoClear, Category = "Item Container View Model")
	TSubclassOf<UItemViewModel> ItemViewModelClass;

	/** Called when a valid ItemContainer is set. */
	virtual void OnItemContainerSet() {}

	/** Called whenever an item is added to the ItemContainer. */
	virtual void OnItemAdded(const FItemInstance& ItemInstance, UItemViewModel* ItemViewModel) {}
	/** Called whenever an item is removed from the ItemContainer. */
	virtual void OnItemRemoved(const FItemInstance& ItemInstance, UItemViewModel* ItemViewModel) {}
	/** Called whenever an item is changed in the ItemContainer. */
	virtual void OnItemChanged(const FItemInstance& ItemInstance, UItemViewModel* ItemViewModel) {}

	/**
	 * Creates an ItemViewModel based on the Item's ItemFragment_UI. If it is invalid, falls back to the 
	 * ItemViewModelClass defined in this class.
	 */
	UItemViewModel* CreateItemViewModel(const FItemInstance& ItemInstance);

	static bool DoesItemHaveUIFragment(const TInstancedStruct<FItem>& Item);

private:
	UPROPERTY()
	TWeakObjectPtr<UItemContainer> WeakItemContainer;

	/** The created ItemViewModels. */
	UPROPERTY()
	TArray<TObjectPtr<UItemViewModel>> ItemViewModels;

	/** Temporarily updated as items are added, removed, and changed. */
	UPROPERTY()
	TObjectPtr<UItemViewModel> ItemViewModelBuffer;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = true))
	FText ItemContainerName;

	void Internal_OnItemAdded(UItemContainer* InContainer, const FItemInstance& ItemInstance);
	void Internal_OnItemRemoved(UItemContainer* InContainer, const FItemInstance& ItemInstance);
	void Internal_OnItemChanged(UItemContainer* InContainer, const FItemInstance& ItemInstance);
};
