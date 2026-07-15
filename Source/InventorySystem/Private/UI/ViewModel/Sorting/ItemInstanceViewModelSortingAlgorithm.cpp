// Copyright Soccertitan 2026


#include "UI/ViewModel/Sorting/ItemInstanceViewModelSortingAlgorithm.h"

UItemInstanceViewModelSortingAlgorithm::UItemInstanceViewModelSortingAlgorithm()
{
}

bool UItemInstanceViewModelSortingAlgorithm::GetResult(const UItemInstanceViewModel* A, const UItemInstanceViewModel* B) const
{
	if (!A || !B)
	{
		if (A)
		{
			return true;
		}
		return false;
	}
	
	return CalculateResult(A, B);
}
