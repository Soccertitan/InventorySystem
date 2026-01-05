// Copyright Soccertitan 2025


#include "InventoryGameplayTags.h"

#include "GameplayTagsManager.h"

FInventoryGameplayTags FInventoryGameplayTags::GameplayTags;

void FInventoryGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.ItemContainer_Default = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ItemContainer.Default"), FString("The default ItemContainerTag for an ItemContainer."));
	
	GameplayTags.ItemAddResult_Error = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ItemAddResult.Error"), FString("Root Gameplay Tag for when an Item can't be fully added to the ItemContainer."));
	
	GameplayTags.ItemAddResult_Error_InvalidItem = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ItemAddResult.Error.InvalidItem"), FString("The item is invalid/nullptr."));
	GameplayTags.ItemAddResult_Error_InvalidQuantity = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ItemAddResult.Error.InvalidQuantity"), FString("The quantity is less than or equal to zero."));
	GameplayTags.ItemAddResult_Error_ContainerNotFound = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ItemAddResult.Error.ContainerNotFound"), FString("Could not find a valid ItemContainer."));
	GameplayTags.ItemAddResult_Error_CantContainItem = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ItemAddResult.Error.CantContainItem"), FString("The item is not allowed to be in the ItemContainer."));
	GameplayTags.ItemAddResult_Error_MaxStacksReached = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ItemAddResult.Error.MaxStacksReached"), FString("The container has reached maximum amount of stacks for the Item, ItemContainer, or InventoryManager."));
}
