// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "ItemInstanceViewModelSorting.h"
#include "ItemInstanceViewModelSorting_Quantity.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEM_API UItemInstanceViewModelSorting_Quantity : public UItemInstanceViewModelSorting
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	bool bDescendingOrder = true;
	
protected:
	virtual bool CalculateResult_Implementation(const UObject* Context, const UItemInstanceViewModel* A, const UItemInstanceViewModel* B) const override;
};
