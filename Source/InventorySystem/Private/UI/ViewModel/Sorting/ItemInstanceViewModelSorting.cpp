// Copyright Soccertitan 2026


#include "UI/ViewModel/Sorting/ItemInstanceViewModelSorting.h"

UItemInstanceViewModelSorting::UItemInstanceViewModelSorting()
{
}

bool UItemInstanceViewModelSorting::GetResult(const UObject* Context, const UItemInstanceViewModel* A, const UItemInstanceViewModel* B) const
{
	if (!A || !B)
	{
		if (A)
		{
			return true;
		}
		return false;
	}
	
	return CalculateResult(Context, A, B);
}
