// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventorySystemInterface.generated.h"

class UInventoryManagerComponent;

// This class does not need to be modified.
UINTERFACE()
class UInventorySystemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Actors that implement this interface will return a InventoryManagerComponent.
 */
class INVENTORYSYSTEM_API IInventorySystemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// Returns the ItemManagerComponent that can live on an actor such as a pawn or PlayerState.
	UFUNCTION(BlueprintNativeEvent)
	UInventoryManagerComponent* GetInventoryManagerComponent() const;
};
