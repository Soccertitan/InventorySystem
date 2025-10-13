// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemContainerRule.generated.h"

class UItemContainer;

/**
 * Abstract class for all ItemContainerRule types. Provides accessor functions that can be used in child classes.
 * It is also expected that the rules are instanced by the ItemContainer.
 */
UCLASS(Abstract, Blueprintable, NotBlueprintType, DefaultToInstanced, EditInlineNew)
class INVENTORYSYSTEM_API UItemContainerRule : public UObject
{
	GENERATED_BODY()

public:

	/** Retrieves the outer of the ItemContainerRule and casts to the passed in ItemContainerClass. */
	UFUNCTION(BlueprintPure, meta = (DeterminesOutputType=ItemContainerClass), Category = "Inventory System|Rule", DisplayName = "GetItemContainer")
	UItemContainer* K2_GetItemContainer(const TSubclassOf<UItemContainer> ItemContainerClass) const;

	/** Retrieves the outer of the ItemContainerRule and casts to the passed in type. */
	template<class T>
	T* GetItemContainer() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, const UItemContainer>::Value,
			"'T' template parameter to GetItemContainer must be derived from UItemContainer");
		return (T*)GetOuter();
	}
};
