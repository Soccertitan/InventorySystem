// Copyright Soccertitan


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

	const FItem* ItemPtr = ItemInstance.Item.GetPtr<FItem>();
	Guid = ItemInstance.GetGuid();
	ItemDefinition = ItemPtr->GetItemDefinition();

	FMemoryWriter MemWriter(ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	ItemInstance.Item.Serialize(Ar);
}

FItemContainerSaveData::FItemContainerSaveData(const FItemContainerInstance& ItemContainerInstance)
{
	if (!ItemContainerInstance.GetItemContainer())
	{
		return;
	}

	ContainerTag = ItemContainerInstance.GetItemContainerTag();
	ContainerClass = ItemContainerInstance.GetItemContainer()->GetClass();

	FMemoryWriter MemWriter(ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	ItemContainerInstance.GetItemContainer()->Serialize(Ar);

	for (const FItemInstance& ItemInstance : ItemContainerInstance.GetItemContainer()->GetItems())
	{
		InventoryItemsSaveData.Add(*const_cast<FItemInstance*>(&ItemInstance));
	}
}
