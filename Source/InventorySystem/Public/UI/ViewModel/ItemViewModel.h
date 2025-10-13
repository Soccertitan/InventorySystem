// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "MVVMViewModelBase.h"
#include "Engine/StreamableManager.h"
#include "ItemViewModel.generated.h"


/**
 * A base implementation of the ItemViewModel. It will load the ItemDefinition for you, ensure you update your project's
 * AssetManager settings to include the ItemDefinition as discoverable.
 */
UCLASS()
class INVENTORYSYSTEM_API UItemViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UItemViewModel();

	/** Initializes the ViewModel with the specified ItemInstance. */
	void SetItemInstance(const FItemInstance& ItemInstance);

	UFUNCTION(BlueprintPure, Category = "Inventory System|View Model")
	FGuid GetGuid() const { return Guid; }
	UFUNCTION(BlueprintPure, Category = "Inventory System|View Model")
	const TInstancedStruct<FItem>& GetItem() const { return Item; }

	FText GetItemName() const {return ItemName;}
	FText GetDescription() const {return Description;}
	UTexture2D* GetIcon() const {return Icon;}
	int32 GetQuantity() const {return Quantity;}
	int32 GetMaxQuantity() const {return MaxQuantity;}

	UFUNCTION(BlueprintPure, FieldNotify)
	bool CanHaveMaxQuantityGreaterThanOne() const {return MaxQuantity > 1;}
	UFUNCTION(BlueprintPure, FieldNotify)
	bool IsAtMaxQuantity() const {return GetQuantity() >= GetMaxQuantity();}
	
	bool GetIsItemDefinitionLoading() const { return bItemDefinitionLoading; }
	bool GetIsItemValid() const { return bItemValid; }

	UInventoryManagerComponent* GetInventoryManagerComponent() const { return InventoryManagerComponent; }
	UItemContainer* GetItemContainer() const { return ItemContainer; }

	/**
	 * Creates a Widget and initializes it with this ViewModel. If the passed in Widget class is null, the function
	 * will retrieve the value in the ItemFragment_UI ItemWidgetClass variable.
	 * @param OwningPlayer The PlayerController that owns the widget.
	 * @param WidgetClass The widget to create.
	 * @return The newly created widget.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|View Model")
	UUserWidget* CreateItemDetailsWidget(APlayerController* OwningPlayer, TSubclassOf<UUserWidget> WidgetClass = nullptr);

	/** Loads the ItemDefinition using the cached copy of the item. */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|View Model")
	void LoadItemDefinition();

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

	/** Called from SetItem when a valid Item has been set. */
	virtual void OnItemSet(const FItemInstance& ItemInstance);

	/** Called from SetItem when a valid Item has been set. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnItemSet")
	void K2_OnItemSet(const FItemInstance& ItemInstance);

	/** Called when the ItemDefinition is loaded. */
	virtual void OnItemDefinitionLoaded(const UItemDefinition* ItemDefinition);

	/** Called when the ItemDefinition is loaded. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnItemDefinitionLoaded")
	void K2_OnItemDefinitionLoaded(const UItemDefinition* ItemDefinition);

	void SetItemName(FText InValue);
	void SetDescription(FText InValue);
	void SetIcon(UTexture2D* InValue);
	void SetQuantity(int32 InValue);
	void SetMaxQuantity(int32 InValue);

private:
	/** Cached identifier of the ItemInstance. */
	UPROPERTY()
	FGuid Guid;
	/** Cached copy of the Item. */
	UPROPERTY()
	TInstancedStruct<FItem> Item;
	/** Cached widget to spawn for the ItemDetailsWidget. */
	UPROPERTY()
	TSoftClassPtr<UUserWidget> ItemWidgetClass;
	/** The owner where the item originated from. */
	UPROPERTY()
	TObjectPtr<UInventoryManagerComponent> InventoryManagerComponent;
	/** The ItemContainer the item is 'stored' in. */
	UPROPERTY()
	TObjectPtr<UItemContainer> ItemContainer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 MaxQuantity = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetIsItemValid, meta = (AllowPrivateAccess = "true"))
	bool bItemValid = false;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetIsItemDefinitionLoading, meta = (AllowPrivateAccess = "true"))
	bool bItemDefinitionLoading = false;

	/** Cached handle for the ItemDefinition. */
	TSharedPtr<FStreamableHandle> ItemDefinitionStreamableHandle;

	/** Called when the ItemDefinition is loaded. */
	void Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UItemDefinition> ItemDefinition);
};
