// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "ItemContainerViewModel.generated.h"

class UItemInstanceViewModelSortingPreset;
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

	UFUNCTION(BlueprintPure, Category = "Viewmodel|ItemContainer")
	UItemContainer* GetItemContainer() const;
	
	FText GetItemContainerName() const {return ItemContainerName;}
	
	bool IsAutoSortEnabled() const { return bAutoSort; }
	void SetAutoSortEnabled(bool bEnabled);
	
	UItemInstanceViewModelSortingPreset* GetAutoSortPreset() const { return AutoSortPreset; }
	void SetAutoSortPreset(UItemInstanceViewModelSortingPreset* Value);
	
	/** Returns true if the ViewModels were sorted. */
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemContainer")
	bool SortItemInstances(const UObject* Context, const UItemInstanceViewModelSortingPreset* SortingPreset);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemContainer")
	int32 GetConsumedCapacity() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemContainer")
	int32 GetMaxCapacity() const;

	/** Called only once when the ItemInstanceViewModels are first created in SetItemContainer. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemContainer", DisplayName = GetItemInstanceViewModels)
	TArray<UItemInstanceViewModel*> K2_GetItemInstanceViewModels() const;
	
	/** Returns a const reference to the array of ItemInstanceViewModels. */
	const TArray<UItemInstanceViewModel*>& GetItemInstanceViewModels() const;

	/** Called whenever an ItemInstanceViewModel is added to the ItemInstanceViewModels. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemContainer")
	UItemInstanceViewModel* GetAddedItemInstanceViewModel() const {return ItemInstanceViewModelBuffer;}

	/** Called whenever an ItemInstanceViewModel is removed to the ItemInstanceViewModels. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemContainer")
	UItemInstanceViewModel* GetRemovedItemInstanceViewModel() const {return ItemInstanceViewModelBuffer;}

protected:
	virtual TSubclassOf<UItemInstanceViewModel> GetItemInstanceViewModelClass(const FItemInstance& ItemInstance) const;
	virtual UObject* GetAutoSortContextObject() const {return nullptr;}
	
	/** Updates the ItemContainer for this ViewModel. Triggers OnItemContainerSet if a new one is set. */
	void SetItemContainer(UItemContainer* InItemContainer);

	/** Called when a valid ItemContainer is set. */
	virtual void OnItemContainerSet() {}

	/** Called whenever an item is added to the ItemContainer. ItemDefs will be loaded. */
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
	
	// Will automatically sort the ItemInstanceViewModels using the AutoSortPreset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter="SetAutoSortEnabled", Getter="IsAutoSortEnabled", meta = (AllowPrivateAccess))
	bool bAutoSort = false;
	
	// The sorting method to use when auto sort is enabled.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	TObjectPtr<UItemInstanceViewModelSortingPreset> AutoSortPreset;

	void OnItemAddedInternal(const FItemInstance& ItemInstance);
	void OnItemRemovedInternal(const FItemInstance& ItemInstance);
	void OnItemChangedInternal(const FItemInstance& ItemInstance);
	
	friend class UInventoryUISubsystem;
};
