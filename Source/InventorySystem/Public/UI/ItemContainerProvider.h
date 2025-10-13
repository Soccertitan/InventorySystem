// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "ItemContainerProvider.generated.h"

class UItemContainer;

/** Context passed into an ItemContainerProvider. */
USTRUCT(BlueprintType)
struct FItemContainerViewContext
{
	GENERATED_BODY()

	/** The user widget requesting the container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	const UUserWidget* UserWidget = nullptr;

	/** Additional context tags. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ContextTags;
};

/**
 * Provides an ItemContainer, for use in view model resolvers or similar situations.
 */
UCLASS(BlueprintType, Blueprintable, Abstract, Const)
class INVENTORYSYSTEM_API UItemContainerProvider : public UObject
{
	GENERATED_BODY()

public:
	/** Returns the relevant InventoryContainer. */
	UFUNCTION(BlueprintNativeEvent)
	UItemContainer* ProvideItemContainer(FGameplayTag ItemContainerTag, FItemContainerViewContext Context) const;
};

/**
 * Provides a game InventoryContainer from the widget's owning player.
 * Checks the player pawn, player state, and player controller in that order.
 */
UCLASS(DisplayName = "Player")
class UItemContainerProvider_Player : public UItemContainerProvider
{
	GENERATED_BODY()

public:
	virtual UItemContainer* ProvideItemContainer_Implementation(FGameplayTag ItemContainerTag, FItemContainerViewContext Context) const override;
};
