// Copyright Soccertitan


#include "InventoryTypes.h"

#include "InventoryFastTypes.h"


int32 FQuantityLimit::GetMaxQuantity() const
{
	return bLimitQuantity ? MaxQuantity : MAX_int32;
}

void FQuantityLimit::SetMaxQuantity(int32 Quantity)
{
	MaxQuantity = FMath::Max(1, Quantity);
	bLimitQuantity = true;
}

void FQuantityLimit::SetLimitQuantity(bool bValue)
{
	bLimitQuantity = bValue;
}

bool FAddItemPlanEntry::IsValid() const
{
	if (QuantityToAdd <= 0)
	{
		return false;
	}

	if (ItemInstance == nullptr
		&& !NewItem.IsValid())
	{
		return false;
	}
	
	return true;
}

bool FAddItemPlan::IsValid() const
{
	if (Entries.Num() == 0)
	{
		return false;
	}

	for (const FAddItemPlanEntry& Entry : Entries)
	{
		if (!Entry.IsValid())
		{
			return false;
		}
	}
	return true;
}

void FAddItemPlan::AddEntry(const FAddItemPlanEntry& InEntry)
{
	if (!InEntry.IsValid())
	{
		return;
	}

	if (InEntry.QuantityToAdd > (AmountToGive - AmountGiven))
	{
		return;
	}

	UpdateAmountGiven(InEntry.QuantityToAdd);
	Entries.Add(InEntry);
}

const TArray<FAddItemPlanEntry>& FAddItemPlan::GetEntries() const
{
	return Entries;
}

void FAddItemPlan::UpdateAmountGiven(int32 NewValue)
{
	AmountGiven = AmountGiven + NewValue;

	if (AmountGiven >= AmountToGive)
	{
		Result = EAddItemResult::AllItemsAdded;
	}
	else if (AmountGiven <= 0)
	{
		Result = EAddItemResult::NoItemsAdded;
	}
	else
	{
		Result = EAddItemResult::SomeItemsAdded;
	}
}

FAddItemPlanResult::FAddItemPlanResult(const FAddItemPlan& InPlan, const TArray<FGuid>& InItemGuids)
{
	AmountToGive = InPlan.GetAmountToGive();
	AmountGiven = InPlan.GetAmountGiven();
	Result = InPlan.GetResult();
	Error = InPlan.Error;
	ItemGuids = InItemGuids;
}

//----------------------------------------------------------------------------------------
// GameplayTagStack
//----------------------------------------------------------------------------------------
FString FGameplayTagStack::ToDebugString() const
{
	return FString::Printf(TEXT("%sx%d"), *Tag.ToString(), Count);
}

const FGameplayTag& FGameplayTagStack::GetTag() const
{
	return Tag;
}

int32 FGameplayTagStack::GetCount() const
{
	return Count;
}

//----------------------------------------------------------------------------------------
// GameplayTagStackContainer
//----------------------------------------------------------------------------------------

void FGameplayTagStackContainer::AddStack(FGameplayTag Tag, int32 DeltaCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to AddStack"), ELogVerbosity::Warning);
		return;
	}

	if (DeltaCount == 0)
	{
		FFrame::KismetExecutionMessage(TEXT("Unable to add 0 to AddStack"), ELogVerbosity::Warning);
		return;
	}

	for (auto It = Items.CreateIterator(); It; ++It)
	{
		FGameplayTagStack& Stack = *It;
		
		if (Stack.Tag == Tag)
		{
			if (Stack.Count + DeltaCount == 0)
			{
				// remove the tag entirely since the value is 0.
				It.RemoveCurrent();
			}
			else
			{
				Stack.Count += DeltaCount;
			}
			return;
		}
	}

	Items.Add(FGameplayTagStack(Tag, DeltaCount));
}

void FGameplayTagStackContainer::SubtractStack(FGameplayTag Tag, int32 DeltaCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to SubtractStack"), ELogVerbosity::Warning);
		return;
	}

	if (DeltaCount == 0)
	{
		FFrame::KismetExecutionMessage(TEXT("Unable to subtract 0 to SubtractStack"), ELogVerbosity::Warning);
		return;
	}

	for (auto It = Items.CreateIterator(); It; ++It)
	{
		FGameplayTagStack& Stack = *It;
		if (Stack.Tag == Tag)
		{
			if (Stack.Count - DeltaCount == 0)
			{
				// remove the tag entirely since the value is 0.
				It.RemoveCurrent();
			}
			else
			{
				// decrease the stack count
				Stack.Count -= DeltaCount;
			}
			return;
		}
	}
}

void FGameplayTagStackContainer::RemoveStack(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to RemoveStack"), ELogVerbosity::Warning);
		return;
	}

	for (auto It = Items.CreateIterator(); It; ++It)
	{
		FGameplayTagStack& Stack = *It;
		if (Stack.Tag == Tag)
		{
			It.RemoveCurrent();
			return;
		}
	}
}

int32 FGameplayTagStackContainer::GetStackCount(FGameplayTag Tag) const
{
	for (const auto& Item : Items)
	{
		if (Item.Tag.MatchesTagExact(Tag))
		{
			return Item.Count;
		}
	}
	return 0;
}

FString FGameplayTagStackContainer::ToDebugString() const
{
	TArray<FString> StackStrings;
	for (const FGameplayTagStack& Stack : Items)
	{
		StackStrings.Add(Stack.ToDebugString());
	}
	return FString::Join(StackStrings, TEXT(", "));
}

const TArray<FGameplayTagStack>& FGameplayTagStackContainer::GetItems() const
{
	return Items;
}

bool FGameplayTagStackContainer::ContainsTag(FGameplayTag Tag, bool bExactMatch) const
{
	for (const auto& Item : Items)
	{
		if (bExactMatch)
		{
			if (Item.Tag.MatchesTagExact(Tag))
			{
				return true;
			}
		}
		else
		{
			if(Item.Tag.MatchesTag(Tag))
			{
				return true;
			}	
		}
	}
	return false;
}

void FGameplayTagStackContainer::Empty()
{
	Items.Empty();
}
