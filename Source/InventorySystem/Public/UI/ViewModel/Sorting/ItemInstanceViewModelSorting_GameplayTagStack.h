// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemInstanceViewModelSorting.h"
#include "ItemInstanceViewModelSorting_GameplayTagStack.generated.h"

/**
 * Sorts the items based on a GameplayTagStack count.
 */
UCLASS()
class INVENTORYSYSTEM_API UItemInstanceViewModelSorting_GameplayTagStack : public UItemInstanceViewModelSorting
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FGameplayTag> TagsToCheck;
	
	UPROPERTY(EditAnywhere)
	bool bDescendingOrder = true;
	
protected:
	virtual bool CalculateResult_Implementation(const UObject* Context, const UItemInstanceViewModel* A, const UItemInstanceViewModel* B) const override;
};
