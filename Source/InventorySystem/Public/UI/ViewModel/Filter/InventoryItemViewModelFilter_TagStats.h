// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "InventoryItemViewModelFilterBase.h"
#include "InventoryItemViewModelFilter_TagStats.generated.h"

/**
 * Filters out items that do not meet the specified GameplayTagStackContainer requirement.
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryItemViewModelFilter_TagStats : public UInventoryItemViewModelFilterBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagStackContainer RequiredTagStats;

protected:

	virtual bool ShouldBeginFilter(const UObject* Context, TArray<UItemViewModel*>& ItemViewModels) const override;
	virtual bool DoesItemViewModelPassFilter(const UObject* Context, UItemViewModel* ItemViewModel) const override;
};
