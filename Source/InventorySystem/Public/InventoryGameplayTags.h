// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

/**
 * Singleton containing native gameplay tags.
 */

struct INVENTORYSYSTEM_API FInventoryGameplayTags
{
	static const FInventoryGameplayTags& Get() {return GameplayTags;}
	static void InitializeNativeGameplayTags();

private:
	static FInventoryGameplayTags GameplayTags;
	
public:

	FGameplayTag ItemContainer_Default;
	
	/**
	 * Generic Root Gameplay Tags
	 */
	FGameplayTag ItemAddResult_Error;
	
	/**
	 * Add Item Plan Errors
	 */
	FGameplayTag ItemAddResult_Error_InvalidItem;
	FGameplayTag ItemAddResult_Error_ContainerNotFound;
	FGameplayTag ItemAddResult_Error_CantContainItem;
	FGameplayTag ItemAddResult_Error_MaxStacksReached;
};
