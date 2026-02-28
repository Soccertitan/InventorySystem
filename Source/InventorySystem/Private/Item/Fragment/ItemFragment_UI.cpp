// Copyright Soccertitan 2025


#include "Item/Fragment/ItemFragment_UI.h"

#include "UI/ViewModel/ItemInstanceViewModel.h"


FItemFragment_UI::FItemFragment_UI()
{
	ItemInstanceViewModelClass = UItemInstanceViewModel::StaticClass();
}

void FItemFragment_UI::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	FItemFragment::GetAssetRegistryTags(Context);

	UObject::FAssetRegistryTag RegistryTag;
	RegistryTag.Type = UObject::FAssetRegistryTag::TT_Alphabetical;
	RegistryTag.Name = "ItemFragment_UI";
	RegistryTag.Value = "true";
	Context.AddTag(RegistryTag);
}
