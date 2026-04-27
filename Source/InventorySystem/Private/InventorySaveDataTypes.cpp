// Copyright Soccertitan 2025


#include "InventorySaveDataTypes.h"

#include "ItemContainer/ItemContainer.h"
#include "Item/ItemDefinition.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

FItemSaveData::FItemSaveData(FItemInstance& ItemInstance)
{
	if (!ItemInstance.IsValid())
	{
		return;
	}

	const FItem* ItemPtr = ItemInstance.GetItem().GetPtr<FItem>();
	Guid = ItemInstance.GetGuid();
	ItemDefinition = ItemPtr->GetItemDefinition();
	Quantity = ItemInstance.GetQuantity();

	FMemoryWriter MemWriter(ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	ItemInstance.GetItemPtr()->Serialize(Ar);
}

FItemContainerSaveData::FItemContainerSaveData(UItemContainer* ItemContainer)
{
	if (!ItemContainer)
	{
		return;
	}

	ContainerTag = ItemContainer->GetItemContainerTag();
	ContainerClass = ItemContainer->GetClass();

	FMemoryWriter MemWriter(ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	ItemContainer->Serialize(Ar);

	for (const FItemInstance& ItemInstance : ItemContainer->K2_GetItems())
	{
		InventoryItemsSaveData.Add(*const_cast<FItemInstance*>(&ItemInstance));
	}
}
