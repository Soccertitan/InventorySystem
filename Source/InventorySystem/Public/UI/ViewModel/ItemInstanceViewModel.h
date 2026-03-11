// Copyright 2025 Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "InventoryFastTypes.h"
#include "MVVMViewModelBase.h"
#include "Engine/StreamableManager.h"
#include "Types/MVVMEventField.h"
#include "ItemInstanceViewModel.generated.h"


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
	
	/** Updates the ViewModel with the specified ItemInstance. */
	void SetItemInstance(const FItemInstance& InItemInstance);
	
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemInstance")
	FMVVMEventField OnViewModelInitialized() const { return{}; }

	UFUNCTION(BlueprintPure, Category = "Viewmodel|ItemInstance")
	const FItemInstance& GetItemInstance() const { return ItemInstance; }
	
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemInstance")
	bool IsValidItemInstance() const { return ItemInstance.IsValid(); }

	FText GetItemName() const { return ItemName; }
	FText GetDescription() const { return Description; }
	TSoftObjectPtr<UTexture2D> GetIcon() const {return Icon;}
	int32 GetQuantity() const {return Quantity;}
	int32 GetMaxQuantity() const {return MaxQuantity;}

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemInstance")
	bool CanHaveMaxQuantityGreaterThanOne() const {return MaxQuantity > 1;}
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemInstance")
	bool IsAtMaxQuantity() const {return Quantity >= MaxQuantity;}
	
	/**
	 * Creates a Widget and initializes it with this ViewModel. If the passed in Widget class is null, the function
	 * will retrieve the value in the ItemFragment_UI ItemWidgetClass variable.
	 * @param OwningPlayer The PlayerController that owns the widget.
	 * @param WidgetClass The widget to create.
	 * @return The newly created widget.
	 */
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	UUserWidget* CreateItemDetailsWidget(APlayerController* OwningPlayer, 
		UPARAM(meta = (MustImplement = "/Script/InventorySystem.ItemViewModelInterface")) TSubclassOf<UUserWidget> WidgetClass = nullptr);
	
	/** Loads the ItemDefinition using the cached copy of the item. */
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	void LoadItemDefinition();
	
	/** Resets the ItemDefinition handle freeing resources. */
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	void ReleaseItemDefinitionHandle();
	
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
	
	/** Called from SetItemInstance when a valid ItemInstance has been set. */
	virtual void OnItemInstanceSet(){}

	/** Called from SetItemInstance when a valid ItemInstance has been set. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnItemInstanceSet")
	void K2_OnItemInstanceSet();

	/** Called when the ItemDefinition is loaded. */
	virtual void OnItemDefinitionLoaded(const UItemDefinition* ItemDefinition){}

	/** Called when the ItemDefinition is loaded. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnItemDefinitionLoaded")
	void K2_OnItemDefinitionLoaded(const UItemDefinition* ItemDefinition);
	
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
	/** Cached copy of the ItemInstance. */
	UPROPERTY()
	FItemInstance ItemInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 MaxQuantity = 0;
	
	FDelegateHandle OnItemInstanceChangedHandle;
	/** Cached handle for the ItemDefinition. */
	TSharedPtr<FStreamableHandle> ItemDefinitionStreamableHandle;
	
	/** Called when the ItemDefinition is loaded. */
	void Internal_OnItemDefinitionLoaded();
};
