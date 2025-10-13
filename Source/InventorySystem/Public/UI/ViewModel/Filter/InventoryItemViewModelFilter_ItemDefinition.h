// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemViewModelFilterBase.h"
#include "InventoryItemViewModelFilter_ItemDefinition.generated.h"

class UItemDefinition;
/**
 * Filters out ViewModels that do not have one of the specified ItemDefinitions.
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryItemViewModelFilter_ItemDefinition : public UInventoryItemViewModelFilterBase
{
	GENERATED_BODY()

public:
	/**
	 * ItemDefinitions to check.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<UItemDefinition>> ItemDefinitions;

protected:
	virtual bool ShouldBeginFilter(const UObject* Context, TArray<UItemViewModel*>& ItemViewModels) const override;

	virtual bool DoesItemViewModelPassFilter(const UObject* Context, UItemViewModel* ItemViewModel) const override;
};
