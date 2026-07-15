// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "ItemInstanceViewModelSortingAlgorithm.h"
#include "ItemInstanceViewModelSorting_Quantity.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEM_API UItemInstanceViewModelSorting_Quantity : public UItemInstanceViewModelSortingAlgorithm
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	bool bDescendingOrder = true;
	
protected:
	virtual bool CalculateResult_Implementation(const UItemInstanceViewModel* A, const UItemInstanceViewModel* B) const override;
};
