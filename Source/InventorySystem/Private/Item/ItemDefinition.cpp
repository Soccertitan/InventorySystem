// Copyright Soccertitan


#include "Item/ItemDefinition.h"

#include "UObject/AssetRegistryTagsContext.h"

void FItemFragment::AddItemShard(TInstancedStruct<FItem>& Item, const TInstancedStruct<FItemShard>& Shard)
{
	FItem* ItemPtr = Item.GetMutablePtr<FItem>();
	ItemPtr->Shards.Add(Shard);
}

UItemDefinition::UItemDefinition()
{
	ItemClass.InitializeAsScriptStruct(FItem::StaticStruct());
	AssetType = TEXT("ItemDefinition");
}

FPrimaryAssetId UItemDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, GetFName());
}

void UItemDefinition::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	Super::GetAssetRegistryTags(Context);

	// Adding OwnedTags to the AssetRegistry.
	FAssetRegistryTag RegistryTag;
	RegistryTag.Type = FAssetRegistryTag::TT_Hidden;
	RegistryTag.Name = "OwnedTags";
	RegistryTag.Value = OwnedTags.ToString();
	Context.AddTag(RegistryTag);

	for (const TInstancedStruct<FItemFragment>& Fragment : Fragments)
	{
		if (const FItemFragment* Ptr = Fragment.GetPtr<FItemFragment>())
		{
			Ptr->GetAssetRegistryTags(Context);
		}
	}
}
