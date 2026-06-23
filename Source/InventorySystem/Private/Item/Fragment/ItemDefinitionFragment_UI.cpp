// Copyright Soccertitan 2025


#include "Item/Fragment/ItemDefinitionFragment_UI.h"

#include "UI/ViewModel/ItemInstanceViewModel.h"


FItemDefinitionFragment_UI::FItemDefinitionFragment_UI()
{
	ItemInstanceViewModelClass = UItemInstanceViewModel::StaticClass();
}

void FItemDefinitionFragment_UI::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	FItemDefinitionFragment::GetAssetRegistryTags(Context);

	UObject::FAssetRegistryTag RegistryTag;
	RegistryTag.Type = UObject::FAssetRegistryTag::TT_Alphabetical;
	RegistryTag.Name = "ItemFragment_UI";
	RegistryTag.Value = "true";
	Context.AddTag(RegistryTag);
}
