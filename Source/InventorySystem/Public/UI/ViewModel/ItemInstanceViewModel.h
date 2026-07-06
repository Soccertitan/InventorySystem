// Copyright 2025 Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "InventoryFastTypes.h"
#include "MVVMViewModelBase.h"
#include "Component/ItemInstanceComponentViewModel.h"
#include "Engine/StreamableManager.h"
#include "Types/MVVMEventField.h"
#include "ItemInstanceViewModel.generated.h"


class UTexture2D;
struct FMVVMEventField;
class UItemInstanceComponentViewModel;
class UItemDefinitionViewModel;

/**
 * A base implementation of the ItemInstanceViewModel. It will load the ItemDefinition for you,
 * ensure you update your project's AssetManager settings to include the ItemDefinition as discoverable.
 */
UCLASS()
class INVENTORYSYSTEM_API UItemInstanceViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	UItemInstanceViewModel();
	
	/** Updates the ViewModel with the specified ItemInstance. Calls SetItem. */
	void SetItemInstance(const FItemInstance& ItemInstance);
	
	/** Updates the ViewModel with the Item. Calls SetItemDefinition and loads additional bundle information if AutoLoad is true. */
	void SetItem(const TInstancedStruct<FItem>& Item, bool bShouldSetItemDefinition = true);
	
	/** Updates the ViewModel with the information from the ItemDefinition. */
	void SetItemDefinition(const UItemDefinition* ItemDefinition);

	UFUNCTION(BlueprintPure, Category = "Viewmodel|ItemInstance")
	FItemInstanceHandle GetHandle() const { return Handle; }
	
	/** Returns the ItemContainer from the handle. Can be null! */
	UFUNCTION(BlueprintPure, Category = "Viewmodel|ItemInstance")
	UItemContainer* GetItemContainer() const { return Handle.GetItemContainer(); }
	
	UFUNCTION(BlueprintPure, Category = "Viewmodel|ItemInstance")
	const TInstancedStruct<FItem>& GetItem() const { return CachedItem; }

	FText GetItemName() const { return ItemName; }
	FText GetDescription() const { return Description; }
	TSoftObjectPtr<UTexture2D> GetIcon() const {return Icon;}
	int32 GetQuantity() const {return Quantity;}
	int32 GetMaxQuantity() const {return MaxQuantity;}

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemInstance")
	bool CanHaveMaxQuantityGreaterThanOne() const {return MaxQuantity > 1;}
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemInstance")
	bool IsAtMaxQuantity() const {return Quantity >= MaxQuantity;}
	
	/** Call FindItemInstanceComponentViewModel when binding to this notify to retrieve the data. */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemInstance")
	FMVVMEventField ItemInstanceComponentViewModelsUpdated() const { return FMVVMEventField{}; }
	
	UFUNCTION(BlueprintPure, DisplayName = "FindItemInstanceComponentViewModel", Category = "Viewmodel|ItemInstance", meta = (DeterminesOutputType="Class"))
	UItemInstanceComponentViewModel* K2_FindItemInstanceComponentViewModel(UPARAM(meta = (AllowAbstract="false")) TSubclassOf<UItemInstanceComponentViewModel> Class) const;
	
	template<typename T> requires std::derived_from<T, UItemInstanceComponentViewModel>
	T* FindItemInstanceComponentViewModel() const;
	
	/** Loads the ItemDefinition using the cached SoftObjectPointer of the ItemDefinition. */
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	void LoadItemDefinition();
	
	/** Resets the ItemDefinition handle freeing resources. */
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	void ReleaseItemDefinitionHandle();
	
protected:
	/** Bundles to load when asynchronously loading the ItemDefinition. */
	UPROPERTY(EditDefaultsOnly, Category = "AssetManager")
	TArray<FName> Bundles;
	
	/** If true, automatically loads the ItemDefinition with bundles during SetItem. */
	UPROPERTY(EditDefaultsOnly, Category = "AssetManager")
	bool bAutoLoadItemDefinition = true;
	
	/** If true, this will call RecursivelyExpandBundleData and recurse into sub bundles of other primary assets loaded by a bundle reference. */
	UPROPERTY(EditDefaultsOnly, Category = "AssetManager")
	bool bLoadRecursive = true;
	
	/** If true, after the ItemDefinition is loaded and functions have had a chance to get data, will automatically unload the steamable handle. */
	UPROPERTY(EditDefaultsOnly, Category = "AssetManager")
	bool bAutoUnloadItemDefinition = true;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnItemInstanceSet(const FItemInstance& ItemInstance);
	virtual void OnItemInstanceSet_Implementation(const FItemInstance& ItemInstance);
	
	UFUNCTION(BlueprintNativeEvent)
	void OnItemSet(const TInstancedStruct<FItem>& Item);
	virtual void OnItemSet_Implementation(const TInstancedStruct<FItem>& Item);

	UFUNCTION(BlueprintNativeEvent)
	void OnItemDefinitionSet(const UItemDefinition* ItemDefinition);
	virtual void OnItemDefinitionSet_Implementation(const UItemDefinition* ItemDefinition);
	
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	void SetItemName(FText InValue);
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	void SetDescription(FText InValue);
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	void SetIcon(const TSoftObjectPtr<UTexture2D>& InValue);
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	void SetQuantity(int32 InValue);
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	void SetMaxQuantity(int32 InValue);
	
private:
	/** The unique identifier for the item. */
	FItemInstanceHandle Handle;
	
	/** Cached copy of the item. */
	UPROPERTY()
	TInstancedStruct<FItem> CachedItem;

	/** Cached soft object pointer of the Item Def */
	UPROPERTY()
	TSoftObjectPtr<UItemDefinition> ItemDefinitionSoft;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	FText ItemName;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	FText Description;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 Quantity = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 MaxQuantity = 0;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UItemInstanceComponentViewModel>> ItemInstanceComponentViewModels;
	
	/** Cached handle for the ItemDefinition. */
	TSharedPtr<FStreamableHandle> ItemDefinitionStreamableHandle;
	
	/** Called when the ItemDefinition is loaded. */
	void OnItemDefinitionLoadedInternal();
	
	/** Creates a new ItemInstanceComponentViewModel for each ItemDefFragment. */
	void CreateItemInstanceComponentViewModelsInternal(const UItemDefinition* ItemDefinition, bool bReset);
};

/**
 * @return A pointer to the first component that matches the class.
 */
template <typename T> requires std::derived_from<T, UItemInstanceComponentViewModel>
T* UItemInstanceViewModel::FindItemInstanceComponentViewModel() const
{
	return K2_FindItemInstanceComponentViewModel(T::StaticClass());
}
