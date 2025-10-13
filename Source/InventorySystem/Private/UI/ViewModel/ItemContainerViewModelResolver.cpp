// Copyright Soccertitan


#include "UI/ViewModel/ItemContainerViewModelResolver.h"

#include "ItemContainer/ItemContainer.h"
#include "InventorySettings.h"
#include "Blueprint/UserWidget.h"
#include "UI/ItemContainerProvider.h"
#include "UI/InventoryUISubsystem.h"
#include "UI/ViewModel/ItemContainerViewModel.h"

bool UItemContainerViewModelResolver::DoesSupportViewModelClass(const UClass* Class) const
{
	if (Super::DoesSupportViewModelClass(Class))
	{
		if (!Class->IsChildOf(UItemContainerViewModel::StaticClass()))
		{
			return false;
		}
	}
	return true;
}

UItemContainerViewModel* UItemContainerViewModelResolver::GetItemContainerViewModel(const UUserWidget* UserWidget, const UMVVMView* View) const
{
	UItemContainer* ItemContainer = GetItemContainer(UserWidget, View);
	if (IsValid(ItemContainer))
	{
		// Get the view model for the container.
		UInventoryUISubsystem* ItemUISubsystem = UserWidget->GetWorld()->GetSubsystem<UInventoryUISubsystem>();
		return ItemUISubsystem->CreateItemContainerViewModel(ItemContainer);
	}
	return nullptr;
}

UItemContainer* UItemContainerViewModelResolver::GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const
{
	return nullptr;
}

//--------------------------------
// ItemContainerViewModelResolver
//--------------------------------

UItemContainerViewModelResolver_ItemContainerTag::UItemContainerViewModelResolver_ItemContainerTag()
{
	ItemContainerTag = UInventorySettings::GetDefaultItemContainerTag();
}

UObject* UItemContainerViewModelResolver_ItemContainerTag::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget,
	const UMVVMView* View) const
{
	return GetItemContainerViewModel(UserWidget, View);
}

UItemContainer* UItemContainerViewModelResolver_ItemContainerTag::GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const
{
	UInventoryUISubsystem* InventoryUISubsystem = UserWidget->GetWorld()->GetSubsystem<UInventoryUISubsystem>();
	const FItemContainerViewContext Context(UserWidget);
	return InventoryUISubsystem->GetItemContainerFromProvider(Provider, ItemContainerTag, Context);
}
