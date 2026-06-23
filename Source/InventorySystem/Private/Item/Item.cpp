// Copyright Soccertitan 2025


#include "Item/Item.h"

#include "Item/ItemDefinition.h"


bool FItemFragment::IsMatching(const TInstancedStruct<FItemFragment>& OtherFragment) const
{
	return this->StaticStruct() == OtherFragment.GetScriptStruct();
}

FItem::FItem()
{
}

bool FItem::IsMatching(const TInstancedStruct<FItem>& OtherItem) const
{
	if (!OtherItem.IsValid())
	{
		return false;
	}
	
	if (OtherItem.GetPtr<FItem>()->ItemDefinition != ItemDefinition)
	{
		return false;
	}

	if (OtherItem.GetPtr<FItem>()->GameplayTagStackContainer != GameplayTagStackContainer)
	{
		return false;
	}

	if (!AreFragmentsEqual(OtherItem))
	{
		return false;
	}

	return true;
}

bool FItem::AreFragmentsEqual(const TInstancedStruct<FItem>& OtherItem) const
{
	const FItem* OtherItemPtr = OtherItem.GetPtr<FItem>();

	if (Fragments.Num() != OtherItemPtr->Fragments.Num())
	{
		return false;
	}

	for (int32 Index = 0; Index < Fragments.Num(); Index++)
	{
		if (!Fragments[Index].Get<FItemFragment>().IsMatching(OtherItemPtr->Fragments[Index]))
		{
			return false;
		}
	}

	return true;
}

TInstancedStruct<FItemFragment> FItem::FindFragmentByScriptStruct(const UScriptStruct* Struct) const
{
	for (const TInstancedStruct<FItemFragment>& Fragment : Fragments)
	{
		if (Fragment.IsValid() && Fragment.GetScriptStruct()->IsChildOf(Struct))
		{
			return Fragment;
		}
	}
	return TInstancedStruct<FItemFragment>();
}

void FItem::Initialize(const UItemDefinition* InItemDefinition)
{
	ItemDefinition = InItemDefinition->GetPathName();

	for (const TTuple<FGameplayTag, int>& Pair : InItemDefinition->DefaultStats)
	{
		GameplayTagStackContainer.AddStack(Pair.Key, Pair.Value);
	}
}
