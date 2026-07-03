// Copyright Soccertitan 2025


#include "Item/ItemDefinition.h"

#include "UI/ViewModel/ItemInstanceViewModel.h"
#include "UObject/AssetRegistryTagsContext.h"


UItemDefinition::UItemDefinition()
{
	ItemInstanceViewModelClass = UItemInstanceViewModel::StaticClass();
	ItemClass.InitializeAsScriptStruct(FItem::StaticStruct());
}

FPrimaryAssetId UItemDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(StaticClass()->GetFName(), GetFName());
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

	for (const TInstancedStruct<FItemDefinitionFragment>& Fragment : Fragments)
	{
		if (const FItemDefinitionFragment* Ptr = Fragment.GetPtr<FItemDefinitionFragment>())
		{
			Ptr->GetAssetRegistryTags(Context);
		}
	}
}
