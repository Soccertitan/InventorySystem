// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventoryItemViewModelFilterBase.generated.h"

class UItemViewModel;

/**
 * An abstract class for filtering ItemViewModels.
 */
UCLASS(Abstract, Blueprintable, DefaultToInstanced, EditInlineNew)
class INVENTORYSYSTEM_API UInventoryItemViewModelFilterBase : public UObject
{
	GENERATED_BODY()

public:
	UInventoryItemViewModelFilterBase();

	/**
	 * Filters out ItemViewModels given a context.
	 * @param Context A context that can contain any information the Filter requires to do its job.
	 * @param ItemViewModels The ItemViewModels to filter.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|Filter", BlueprintPure = false)
	void FilterItemViewModels(const UObject* Context, UPARAM(ref) TArray<UItemViewModel*>& ItemViewModels) const;

protected:
	/**
	 * Determines if the requirements are met to loop through the ItemViewModels.
	 * @param Context A context that can contain any information the Filter requires to do its job.
	 * @param ItemViewModels The ItemViewModels that may be filtered.
	 * @return True, if the requirements are met.
	 */
	virtual bool ShouldBeginFilter(const UObject* Context, TArray<UItemViewModel*>& ItemViewModels) const;

	/**
	 * Determines if the requirements are met to loop through the ItemViewModels.
	 * @param Context A context that can contain any information the Filter requires to do its job.
	 * @param ItemViewModels The ItemViewModels that may be filtered.
	 * @return True, if the requirements are met.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|Filter", meta = (DisplayName = "ShouldBeginFilter"))
	bool K2_ShouldBeginFilter(const UObject* Context, TArray<UItemViewModel*>& ItemViewModels) const;

	/**
	 * Checks the filter criteria and returns a boolean if it passed the filter criteria. The ItemViewModel and Item
	 * are guaranteed to be valid.
	 * @param Context A context that can contain any information the Filter requires to do its job.
	 * @param ItemViewModel The ItemViewModel to evaluate.
	 * @return True, if the ViewModel passes the filter criteria.
	 */
	virtual bool DoesItemViewModelPassFilter(const UObject* Context, UItemViewModel* ItemViewModel) const;

	/**
	 * Checks the filter criteria and returns a boolean if it passed the filter criteria. The ItemViewModel and Item
	 * are guaranteed to be valid.
	 * @param Context A context that can contain any information the Filter requires to do its job.
	 * @param ItemViewModel The ItemViewModel to evaluate.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|Filter", meta = (DisplayName = "DoesItemViewModelPassFilter"))
	bool K2_DoesItemViewModelPassFilter(const UObject* Context, UItemViewModel* ItemViewModel) const;

private:

	uint8 bHasShouldBeginFilter : 1;
	uint8 bHasDoesItemViewModelPassFilter : 1;
};
