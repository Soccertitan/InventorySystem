// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemViewModelInterface.generated.h"

class UItemViewModel;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UItemViewModelInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Designed to allow code to inject a ViewModel into a widget for displaying information about an item.
 */
class INVENTORYSYSTEM_API IItemViewModelInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/**
	 * @param ItemViewModel The ViewModel to assign to the widget.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory System|View Model")
	void SetItemViewModel(UItemViewModel* ItemViewModel);
};
