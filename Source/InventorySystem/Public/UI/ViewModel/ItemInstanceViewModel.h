// Copyright 2025 Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "InventoryFastTypes.h"
#include "MVVMViewModelBase.h"
#include "Engine/StreamableManager.h"
#include "Types/MVVMEventField.h"
#include "ItemInstanceViewModel.generated.h"


class UItemViewModel;
/**
 * A base implementation of the ItemInstanceViewModel. It will load the ItemDefinition for you and create an ItemViewModel, 
 * ensure you update your project's AssetManager settings to include the ItemDefinition as discoverable.
 */
UCLASS()
class INVENTORYSYSTEM_API UItemInstanceViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	UItemInstanceViewModel();
	
	/** Initializes the ViewModel with the specified ItemInstance. */
	void SetItemInstance(const FItemInstance& ItemInstance);
	
	UFUNCTION(BlueprintPure, FieldNotify)
	FMVVMEventField OnViewModelInitialized() const { return{}; }
	
	UFUNCTION(BlueprintPure, Category = "Inventory System|View Model")
	FGuid GetGuid() const { return Guid; }
	UFUNCTION(BlueprintPure, Category = "Inventory System|View Model")
	const TInstancedStruct<FItem>& GetItem() const { return Item; }
	
	UItemViewModel* GetItemViewModel() const { return ItemViewModel; }
	
	int32 GetQuantity() const {return Quantity;}
	int32 GetMaxQuantity() const {return MaxQuantity;}

	UFUNCTION(BlueprintPure, FieldNotify)
	bool CanHaveMaxQuantityGreaterThanOne() const {return MaxQuantity > 1;}
	UFUNCTION(BlueprintPure, FieldNotify)
	bool IsAtMaxQuantity() const {return Quantity >= MaxQuantity;}
	
	UInventoryManagerComponent* GetInventoryManagerComponent() const { return InventoryManagerComponent; }
	UItemContainer* GetItemContainer() const { return ItemContainer; }
	
	/** Loads the ItemDefinition using the cached copy of the item. */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|View Model")
	void LoadItemDefinition();
	
protected:
	
	/** The ItemViewModel to create if the ItemDefinition does not have a valid entry. */
	UPROPERTY(EditDefaultsOnly, NoClear)
	TSubclassOf<UItemViewModel> ItemViewModelClass;
	
	/** Bundles to load when asynchronously loading the ItemDefinition. */
	UPROPERTY(EditDefaultsOnly, Category = "AssetManager")
	TArray<FName> Bundles;
	
	/** If true, automatically loads the ItemDefinition during SetItemInstance. */
	UPROPERTY(EditDefaultsOnly, Category = "AssetManager")
	bool bAutoLoadItemDefinition = true;
	
	/** If true, this will call RecursivelyExpandBundleData and recurse into sub bundles of other primary assets loaded by a bundle reference. */
	UPROPERTY(EditDefaultsOnly, Category = "AssetManager")
	bool bLoadRecursive = true;
	
	/** Called from SetItemInstance when a valid ItemInstance has been set. */
	virtual void OnItemInstanceSet(const FItemInstance& ItemInstance){}

	/** Called from SetItemInstance when a valid ItemInstance has been set. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnItemSet")
	void K2_OnItemInstanceSet(const FItemInstance& ItemInstance);
	
	/** Called when the ItemDefinition is loaded. */
	virtual void OnItemDefinitionLoaded(const UItemDefinition* ItemDefinition){}

	/** Called when the ItemDefinition is loaded. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnItemDefinitionLoaded")
	void K2_OnItemDefinitionLoaded(const UItemDefinition* ItemDefinition);
	
	void SetQuantity(int32 InValue);
	void SetMaxQuantity(int32 InValue);
	
private:
	
	/** Cached identifier of the ItemInstance. */
	UPROPERTY()
	FGuid Guid;
	/** Cached copy of the Item. */
	UPROPERTY()
	TInstancedStruct<FItem> Item;
	/** The owner where the item originated from. */
	UPROPERTY()
	TObjectPtr<UInventoryManagerComponent> InventoryManagerComponent;
	/** The ItemContainer the item is 'stored' in. */
	UPROPERTY()
	TObjectPtr<UItemContainer> ItemContainer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UItemViewModel> ItemViewModel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 MaxQuantity = 0;
	
	/** Cached handle for the ItemDefinition. */
	TSharedPtr<FStreamableHandle> ItemDefinitionStreamableHandle;

	/** Creates the ItemViewModel after the ItemDefinition has loaded. */
	void CreateItemViewModel(const UItemDefinition* ItemDefinition);
	
	/** Called when the ItemDefinition is loaded. */
	void Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UItemDefinition> ItemDefinition);
};
