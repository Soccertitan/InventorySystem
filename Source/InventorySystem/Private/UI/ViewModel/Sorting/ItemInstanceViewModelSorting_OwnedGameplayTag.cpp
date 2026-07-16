// Copyright Soccertitan 2026


#include "UI/ViewModel/Sorting/ItemInstanceViewModelSorting_OwnedGameplayTag.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"

bool UItemInstanceViewModelSorting_OwnedGameplayTag::CalculateResult_Implementation(const UObject* Context, const UItemInstanceViewModel* A, const UItemInstanceViewModel* B) const
{
	if (Super::CalculateResult_Implementation(Context, A, B))
	{
		return true;
	}
	
	const UItemDefinition* DefA = UInventoryBlueprintFunctionLibrary::GetItemDefinition(A->GetItem());
	const UItemDefinition* DefB = UInventoryBlueprintFunctionLibrary::GetItemDefinition(B->GetItem());
	
	if (!DefA || !DefB)
	{
		if (DefA)
		{
			return true;
		}
		return false;
	}
	
	for (const FGameplayTagContainer& Container : OwnedGameplayTagsToCheck)
	{
		bool MatchA = false;
		bool MatchB = false;
		if (bMatchExact)
		{
			MatchA = DefA->OwnedTags.HasAllExact(Container);
			MatchB = DefB->OwnedTags.HasAllExact(Container);
		}
		else
		{
			MatchA = DefA->OwnedTags.HasAll(Container);
			MatchB = DefB->OwnedTags.HasAll(Container);
		}
		
		if (MatchA == MatchB)
		{
			continue;
		}
		if (MatchA)
		{
			return true;
		}
		if (MatchB)
		{
			return false;
		}
	}
	
	return false;
}
