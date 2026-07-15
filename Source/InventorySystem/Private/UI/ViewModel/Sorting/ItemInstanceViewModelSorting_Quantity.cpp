// Copyright Soccertitan 2026


#include "UI/ViewModel/Sorting/ItemInstanceViewModelSorting_Quantity.h"

#include "UI/ViewModel/ItemInstanceViewModel.h"

bool UItemInstanceViewModelSorting_Quantity::CalculateResult_Implementation(const UItemInstanceViewModel* A, const UItemInstanceViewModel* B) const
{
	if (bDescendingOrder)
	{
		return A->GetQuantity() > B->GetQuantity();
	}
	return A->GetQuantity() < B->GetQuantity();
}
