// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemInstanceViewModelSorting.h"
#include "ItemInstanceViewModelSorting_OwnedGameplayTag.generated.h"

/**
 * Checks the ItemDefinition for owned gameplay tags.
 */
UCLASS()
class INVENTORYSYSTEM_API UItemInstanceViewModelSorting_OwnedGameplayTag : public UItemInstanceViewModelSorting
{
	GENERATED_BODY()
	
public:
	/** 
	 * The tags to check as an array order. 
	 * If A has matching tags and B does not. Return true.
	 * If A does not have tag and B does have tag. Return false.
	 * If A does not have tag and B does not have tag. Continue the loop.
	 */
	UPROPERTY(EditAnywhere, meta = (NoResetToDefault))
	TArray<FGameplayTagContainer> OwnedGameplayTagsToCheck;
	
	UPROPERTY(EditAnywhere)
	bool bMatchExact = false;
	
protected:
	virtual bool CalculateResult_Implementation(const UObject* Context, const UItemInstanceViewModel* A, const UItemInstanceViewModel* B) const override;
};
