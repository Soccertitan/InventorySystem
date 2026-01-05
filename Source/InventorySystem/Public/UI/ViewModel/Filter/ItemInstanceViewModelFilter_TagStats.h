// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "ItemInstanceViewModelFilter.h"
#include "ItemInstanceViewModelFilter_TagStats.generated.h"

/**
 * Filters out items that do not meet the specified GameplayTagStackContainer requirement.
 */
UCLASS()
class INVENTORYSYSTEM_API UItemInstanceViewModelFilter_TagStats : public UItemInstanceViewModelFilter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagStackContainer RequiredTagStats;

protected:

	virtual bool ShouldBeginFilter(const UObject* Context, TArray<UItemInstanceViewModel*>& ItemInstanceViewModels) const override;
	virtual bool DoesItemInstanceViewModelPassFilter(const UObject* Context, UItemInstanceViewModel* ItemInstanceViewModel) const override;
};
