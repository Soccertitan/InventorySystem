// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemInstanceViewModelSorting.generated.h"

class UItemInstanceViewModel;

/**
 * Returns true/false for the sorting algorithm.
 */
UCLASS(Abstract, Blueprintable, DefaultToInstanced, EditInlineNew)
class INVENTORYSYSTEM_API UItemInstanceViewModelSorting : public UObject
{
	GENERATED_BODY()
	
public:
	UItemInstanceViewModelSorting();
	
	/** Returns true if A comes before B. */
	bool GetResult(const UObject* Context, const UItemInstanceViewModel* A, const UItemInstanceViewModel* B) const;

protected:
	
	/** Return true if A comes before B. A and B guaranteed to be valid. */
	UFUNCTION(BlueprintNativeEvent)
	bool CalculateResult(const UObject* Context, const UItemInstanceViewModel* A, const UItemInstanceViewModel* B) const;
	
	virtual bool CalculateResult_Implementation(const UObject* Context, const UItemInstanceViewModel* A, const UItemInstanceViewModel* B) const { return false; }
};
