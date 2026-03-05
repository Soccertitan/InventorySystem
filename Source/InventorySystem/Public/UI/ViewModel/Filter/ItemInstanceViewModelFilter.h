// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemInstanceViewModelFilter.generated.h"

class UItemInstanceViewModel;

/**
 * An abstract class for filtering ItemInstanceViewModels.
 */
UCLASS(Abstract, Blueprintable, DefaultToInstanced, EditInlineNew)
class INVENTORYSYSTEM_API UItemInstanceViewModelFilter : public UObject
{
	GENERATED_BODY()

public:
	UItemInstanceViewModelFilter();

	/**
	 * Filters out ItemViewModels given a context.
	 * @param Context A context that can contain any information the Filter requires to do its job.
	 * @param ItemInstanceViewModels The ItemViewModels to filter.
	 */
	UFUNCTION(BlueprintCallable, Category = "Viewmodel|Filter", BlueprintPure = false)
	void FilterItemInstanceViewModels(const UObject* Context, UPARAM(ref) TArray<UItemInstanceViewModel*>& ItemInstanceViewModels) const;

protected:
	/**
	 * Determines if the requirements are met to loop through the ItemInstanceViewModels.
	 * @param Context A context that can contain any information the Filter requires to do its job.
	 * @param ItemInstanceViewModels The ItemInstanceViewModels that may be filtered.
	 * @return True, if the requirements are met.
	 */
	virtual bool ShouldBeginFilter(const UObject* Context, TArray<UItemInstanceViewModel*>& ItemInstanceViewModels) const;

	/**
	 * Determines if the requirements are met to loop through the ItemInstanceViewModels.
	 * @param Context A context that can contain any information the Filter requires to do its job.
	 * @param ItemInstanceViewModels The ItemInstanceViewModels that may be filtered.
	 * @return True, if the requirements are met.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Viewmodel|Filter", meta = (DisplayName = "ShouldBeginFilter"))
	bool K2_ShouldBeginFilter(const UObject* Context, TArray<UItemInstanceViewModel*>& ItemInstanceViewModels) const;

	/**
	 * Checks the filter criteria and returns a boolean if it passed the filter criteria. The ItemInstanceViewModel and 
	 * Item are guaranteed to be valid.
	 * @param Context A context that can contain any information the Filter requires to do its job.
	 * @param ItemInstanceViewModel The ItemViewModel to evaluate.
	 * @return True, if the ViewModel passes the filter criteria.
	 */
	virtual bool DoesItemInstanceViewModelPassFilter(const UObject* Context, UItemInstanceViewModel* ItemInstanceViewModel) const;

	/**
	 * Checks the filter criteria and returns a boolean if it passed the filter criteria. The ItemInstanceViewModel and 
	 * Item are guaranteed to be valid.
	 * @param Context A context that can contain any information the Filter requires to do its job.
	 * @param ItemInstanceViewModel The ItemViewModel to evaluate.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Viewmodel|Filter", meta = (DisplayName = "DoesItemInstanceViewModelPassFilter"))
	bool K2_DoesItemInstanceViewModelPassFilter(const UObject* Context, UItemInstanceViewModel* ItemInstanceViewModel) const;

private:

	uint8 bHasShouldBeginFilter : 1;
	uint8 bHasDoesItemInstanceViewModelPassFilter : 1;
};
