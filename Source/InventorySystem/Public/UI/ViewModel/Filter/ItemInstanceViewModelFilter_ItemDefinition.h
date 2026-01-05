// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "ItemInstanceViewModelFilter.h"
#include "ItemInstanceViewModelFilter_ItemDefinition.generated.h"

class UItemDefinition;

/**
 * Filters out ViewModels that do not have one of the specified ItemDefinitions.
 */
UCLASS()
class INVENTORYSYSTEM_API UItemInstanceViewModelFilter_ItemDefinition : public UItemInstanceViewModelFilter
{
	GENERATED_BODY()

public:
	/**
	 * ItemDefinitions to check.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<UItemDefinition>> ItemDefinitions;

protected:
	virtual bool ShouldBeginFilter(const UObject* Context, TArray<UItemInstanceViewModel*>& ItemInstanceViewModels) const override;

	virtual bool DoesItemInstanceViewModelPassFilter(const UObject* Context, UItemInstanceViewModel* ItemInstanceViewModel) const override;
};
