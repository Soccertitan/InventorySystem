// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemInstanceViewModelFilter.h"
#include "ItemInstanceViewModelFilter_OwnedTags.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEM_API UItemInstanceViewModelFilter_OwnedTags : public UItemInstanceViewModelFilter
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (NoResetToDefault))
	FGameplayTagContainer OwnedGameplayTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMatchExact = false;
	
protected:
	virtual bool ShouldFilterItemInstance(const UObject* Context, UItemInstanceViewModel* ItemInstanceViewModel) const override;
};
