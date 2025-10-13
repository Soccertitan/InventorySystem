// Copyright Soccertitan


#include "Item/ItemSet.h"

#include "Item/ItemDefinition.h"
#include "InventoryManagerComponent.h"
#include "UObject/ObjectSaveContext.h"

const TInstancedStruct<FItem>& FItemSetInstance::GetItem() const
{
	return Item;
}

void FItemSetInstance::TryCreateItem()
{
	if (ItemDefinition)
	{
		if (!Item.IsValid() ||
			Item.GetPtr<FItem>()->GetItemDefinition() != ItemDefinition)
		{
			Item = UInventoryManagerComponent::CreateItem(ItemDefinition);
			bShowItem = true;
		}
	}
	else
	{
		Item = TInstancedStruct<FItem>();
		bShowItem = false;
	}
}

void FItemSetInstance::PostSerialize(const FArchive& Ar)
{
	EditorDisplayName = FString::Printf(TEXT("%s"), *GetNameSafe(ItemDefinition));
}

UItemSet::UItemSet()
{
}

FPrimaryAssetId UItemSet::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("ItemSet", GetFName());
}

void UItemSet::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(FItemSetInstance, ItemDefinition))
	{
		for (FItemSetInstance& ItemInstance : ItemInstances)
		{
			ItemInstance.TryCreateItem();
		}
	}
}

void UItemSet::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	// Clean up invalid Items from the array.
	for (int32 idx = ItemInstances.Num() - 1; idx >= 0; idx--)
	{
		if (!ItemInstances[idx].Item.IsValid())
		{
			ItemInstances.RemoveAt(idx);
		}
	}
}
