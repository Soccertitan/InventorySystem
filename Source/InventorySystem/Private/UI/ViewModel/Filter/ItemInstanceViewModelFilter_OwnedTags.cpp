// Copyright Soccertitan 2026


#include "UI/ViewModel/Filter/ItemInstanceViewModelFilter_OwnedTags.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"

bool UItemInstanceViewModelFilter_OwnedTags::ShouldFilterItemInstance(const UObject* Context, UItemInstanceViewModel* ItemInstanceViewModel) const
{
	if (Super::ShouldFilterItemInstance(Context, ItemInstanceViewModel))
	{
		return true;
	}
	
	const UItemDefinition* ItemDefinition = UInventoryBlueprintFunctionLibrary::GetItemDefinition(ItemInstanceViewModel->GetItem());
	if (!ItemDefinition)
	{
		return true;
	}
	
	if (bMatchExact)
	{
		return !ItemDefinition->OwnedTags.HasAllExact(OwnedGameplayTags);
	}
	return !ItemDefinition->OwnedTags.HasAll(OwnedGameplayTags);
}
