// Copyright 2025 Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "InventoryFastTypes.h"
#include "MVVMViewModelBase.h"
#include "Engine/StreamableManager.h"
#include "ItemInstanceViewModel.generated.h"


class UItemViewModel;
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

	UFUNCTION(BlueprintPure, Category = "Viewmodel|ItemInstance")
	const FItemInstance& GetItemInstance() const { return ItemInstance; }
	
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Viewmodel|ItemInstance")
	bool IsItemInstanceValid() const { return ItemInstance.IsValid(); }

	UItemViewModel* GetItemViewModel() const { return ItemViewModel; }
	int32 GetQuantity() const {return Quantity;}
	
protected:
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|ItemInstance")
	void SetQuantity(int32 InValue);
	
	void LoadItemDefinition(TSoftObjectPtr<UItemDefinition> ItemDefinition);
	
private:
	/** Cached copy of the ItemInstance. */
	UPROPERTY()
	FItemInstance ItemInstance;

	/** The item details. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UItemViewModel> ItemViewModel;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 Quantity = 0;

	/** Cached handle for the ItemDefinition. */
	TSharedPtr<FStreamableHandle> ItemDefinitionStreamableHandle;

	/** Called when the ItemDefinition is loaded. */
	void Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UItemDefinition> ItemDefinitionSoft);
};
