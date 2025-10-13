// Copyright Soccertitan


#include "UI/InventoryUISubsystem.h"

#include "InventoryManagerComponent.h"
#include "InventoryBlueprintFunctionLibrary.h"
#include "ItemContainer/ItemContainer.h"
#include "Engine/AssetManager.h"
#include "UI/ItemContainerProvider.h"
#include "UI/ViewModel/ItemContainerViewModel.h"

UItemContainerViewModel* UInventoryUISubsystem::CreateItemContainerViewModel(UItemContainer* ItemContainer)
{
	InventoryContainerViewModels.Remove(nullptr);
	
	if (!IsValid(ItemContainer))
	{
		return nullptr;
	}

	for (const TWeakObjectPtr<UItemContainerViewModel>& VM : InventoryContainerViewModels)
	{
		if (IsValid(VM.Get()) && VM.Get()->GetItemContainer() == ItemContainer)
		{
			return VM.Get();
		}
	}

	return CreateItemContainerViewModel(ItemContainer);
}

UItemContainerViewModel* UInventoryUISubsystem::CreateItemContainerViewModelForActor(AActor* Actor, FGameplayTag ItemContainerTag)
{
	if (UInventoryManagerComponent* ItemManager = UInventoryBlueprintFunctionLibrary::GetInventoryManagerComponent(Actor))
	{
		return CreateItemContainerViewModel(ItemManager->FindItemContainerByTag(ItemContainerTag));
	}
	return nullptr;
}

UItemContainer* UInventoryUISubsystem::GetItemContainerFromProvider(TSubclassOf<UItemContainerProvider> Provider,
	FGameplayTag ItemContainerTag, const FItemContainerViewContext& Context)
{
	if (Provider)
	{
		if (const UItemContainerProvider* ProviderCDO = GetDefault<UItemContainerProvider>(Provider))
		{
			return ProviderCDO->ProvideItemContainer(ItemContainerTag, Context);
		}
	}
	return nullptr;
}

UItemContainerViewModel* UInventoryUISubsystem::Internal_CreateContainerViewModel(UItemContainer* Container)
{
	if (!Container->GetViewModelClass().Get())
	{
		UAssetManager::Get().LoadAssetList({Container->GetViewModelClass().ToSoftObjectPath()})->WaitUntilComplete();
	}
	UItemContainerViewModel* NewVM = NewObject<UItemContainerViewModel>(this, Container->GetViewModelClass().Get());
	NewVM->SetItemContainer(Container);
	InventoryContainerViewModels.Add(NewVM);
	return NewVM;
}
