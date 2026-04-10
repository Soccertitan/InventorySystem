// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Engine/StreamableManager.h"
#include "Item/Item.h"
#include "ItemViewModel.generated.h"

struct FItem;
class UItemDefinition;
class UItemInstanceViewModel;

/**
 * Usually created by an ItemInstanceViewModel and updated by the ItemContainerViewModel. Designed to be subclassed
 * to customize information displayed for different items.
 */
UCLASS()
class INVENTORYSYSTEM_API UItemViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	UItemViewModel();
	
	FText GetItemName() const { return ItemName; }
	FText GetDescription() const { return Description; }
	TSoftObjectPtr<UTexture2D> GetIcon() const {return Icon;}
	int32 GetMaxQuantity() const {return MaxQuantity;}
	
	UFUNCTION(BlueprintPure, Category = "Viewmodel|Item")
	UItemInstanceViewModel* GetItemInstanceViewModel() const {return ItemInstanceViewModel;}
	
	UFUNCTION(BlueprintPure, Category = "Viewmodel|Item")
	TSoftObjectPtr<UItemDefinition> GetItemDefinition() const {return ItemDefinitionSoft;}
	
	/** Loads the ItemDefinition using the cached copy of the item. */
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|Item")
	void LoadItemDefinition(TSoftObjectPtr<UItemDefinition> ItemDefinition);
	
	/** Resets the ItemDefinition handle freeing resources. */
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|Item")
	void ReleaseItemDefinitionHandle();
	
	/**
	 * Creates a Widget and initializes it with this ViewModel.
	 * @param OwningPlayer The PlayerController that owns the widget.
	 * @param WidgetClass The widget to create.
	 * @param bForceWidgetClass If true, will create the specified WidgetClass instead of the one defined in the item.
	 * @return The newly created widget.
	 */
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|Item")
	UUserWidget* CreateItemDetailsWidget(APlayerController* OwningPlayer, 
		UPARAM(meta = (MustImplement = "/Script/InventorySystem.ItemViewModelInterface")) TSubclassOf<UUserWidget> WidgetClass = nullptr,
		bool bForceWidgetClass = false);
	
	/** Internally calls SetItem */
	void SetItemInstance(UItemInstanceViewModel* ViewModel);
	
	/** Initializes the ItemViewModel with the given item. May load the ItemDefinition. */
	void SetItem(const TInstancedStruct<FItem>& Item);
	
	/** 
	 * Initializes the viewmodel with the given ItemDefinition. Useful if you don't have a valid item and want to display 
	 * some information.
	 */
	void SetItemDefinition(const UItemDefinition* ItemDefinition);
	
protected:
	/** Bundles to load when asynchronously loading the ItemDefinition. */
	UPROPERTY(EditDefaultsOnly, Category = "AssetManager")
	TArray<FName> Bundles;
	
	/** If true, automatically loads the ItemDefinition during SetItemInstance. */
	UPROPERTY(EditDefaultsOnly, Category = "AssetManager")
	bool bAutoLoadItemDefinition = true;
	
	/** If true, this will call RecursivelyExpandBundleData and recurse into sub bundles of other primary assets loaded by a bundle reference. */
	UPROPERTY(EditDefaultsOnly, Category = "AssetManager")
	bool bLoadRecursive = true;
	
	/** If true, after the ItemDefinition is loaded and functions have had a chance to get data, will automatically unload the resource. */
	UPROPERTY(EditDefaultsOnly, Category = "AssetManager")
	bool bAutoUnloadItemDefinition = true;
	
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|Item")
	bool CanHaveMaxQuantityGreaterThanOne() const {return MaxQuantity > 1;}
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|Item")
	bool IsAtMaxQuantity() const;
	
	/** Called from SetItemInstance when a valid ItemInstance has been set. */
	virtual void OnItemInstanceSet();

	/** Called from SetItemInstance when a valid ItemInstance has been set. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnItemInstanceSet")
	void K2_OnItemInstanceSet();
	
	/** Called from SetItemInstance when a valid ItemInstance has been set. */
	virtual void OnItemSet(const TInstancedStruct<FItem>& Item) {}

	/** Called from SetItemInstance when a valid ItemInstance has been set. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnItemSet")
	void K2_OnItemSet(const TInstancedStruct<FItem>& Item);

	/** Called when the ItemDefinition is loaded. */
	virtual void OnItemDefinitionSet(const UItemDefinition* ItemDefinition);

	/** Called when the ItemDefinition is loaded. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnItemDefinitionSet")
	void K2_OnItemDefinitionSet(const UItemDefinition* ItemDefinition);
	
	void SetItemName(FText InValue);
	void SetDescription(FText InValue);
	void SetIcon(const TSoftObjectPtr<UTexture2D>& InValue);
	void SetMaxQuantity(int32 InValue);
	
private:
	/** The ItemInstance that may have spawned this ItemViewModel. May be null! */
	UPROPERTY()
	TObjectPtr<UItemInstanceViewModel> ItemInstanceViewModel;
	
	UPROPERTY()
	TSoftObjectPtr<UItemDefinition> ItemDefinitionSoft;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	FText ItemName;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	FText Description;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 MaxQuantity = 0;
	
	/** Cached handle for the ItemDefinition. */
	TSharedPtr<FStreamableHandle> ItemDefinitionStreamableHandle;

	void Internal_SetItemDefinition(const UItemDefinition* ItemDefinition);
	
	/** Called when the ItemDefinition is loaded. */
	void Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UItemDefinition> Definition);
};
