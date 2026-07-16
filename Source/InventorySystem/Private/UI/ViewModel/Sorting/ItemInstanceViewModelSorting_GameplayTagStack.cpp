// Copyright Soccertitan 2026


#include "UI/ViewModel/Sorting/ItemInstanceViewModelSorting_GameplayTagStack.h"

#include "UI/ViewModel/ItemInstanceViewModel.h"

bool UItemInstanceViewModelSorting_GameplayTagStack::CalculateResult_Implementation(const UObject* Context, const UItemInstanceViewModel* A, const UItemInstanceViewModel* B) const
{
	if (Super::CalculateResult_Implementation(Context, A, B))
	{
		return true;
	}
	
	const FItem& ItemA = A->GetItem().Get();
	const FItem& ItemB = B->GetItem().Get();
	
	for (const FGameplayTag& Tag : TagsToCheck)
	{
		int32 TagCountA = ItemA.GameplayTagStackContainer.GetStackCount(Tag);
		int32 TagCountB = ItemB.GameplayTagStackContainer.GetStackCount(Tag);
		
		if (bDescendingOrder)
		{
			if (TagCountA > TagCountB)
			{
				return true;
			}
		}
		else
		{
			if (TagCountA < TagCountB)
			{
				return true;
			}
		}
	}
	
	return false;
}
