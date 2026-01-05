// Copyright Soccertitan 2025


#include "UI/ItemContainerProvider.h"

#include "InventoryManagerComponent.h"
#include "InventoryBlueprintFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "ItemContainer/ItemContainer.h"
#include "GameFramework/PlayerState.h"

UItemContainer* UItemContainerProvider::ProvideItemContainer_Implementation(FGameplayTag ItemContainerTag, FItemContainerViewContext Context) const
{
	return nullptr;
}

UItemContainer* UItemContainerProvider_Player::ProvideItemContainer_Implementation(FGameplayTag ItemContainerTag,
	FItemContainerViewContext Context) const
{
	if (!Context.UserWidget)
	{
		return nullptr;
	}

	APlayerController* Player = Context.UserWidget->GetOwningPlayer();
	if (!Player)
	{
		return nullptr;
	}

	UItemContainer* Result = nullptr;

	if (UInventoryManagerComponent* InventoryManagerComponent = UInventoryBlueprintFunctionLibrary::GetInventoryManagerComponent(Player->GetPawn()))
	{
		Result = InventoryManagerComponent->FindItemContainerByTag(ItemContainerTag);
	}

	if (!IsValid(Result))
	{
		if (UInventoryManagerComponent* InventoryManagerComponent = UInventoryBlueprintFunctionLibrary::GetInventoryManagerComponent(Player->PlayerState))
		{
			Result = InventoryManagerComponent->FindItemContainerByTag(ItemContainerTag);
		}
	}

	if (!IsValid(Result))
	{
		if (UInventoryManagerComponent* InventoryManagerComponent = UInventoryBlueprintFunctionLibrary::GetInventoryManagerComponent(Player))
		{
			Result = InventoryManagerComponent->FindItemContainerByTag(ItemContainerTag);
		}
	}

	return Result;
}
