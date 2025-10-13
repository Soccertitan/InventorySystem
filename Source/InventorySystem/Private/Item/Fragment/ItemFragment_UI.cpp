// Copyright Soccertitan


#include "Item/Fragment/ItemFragment_UI.h"

#include "UI/ViewModel/ItemViewModel.h"


FItemFragment_UI::FItemFragment_UI()
{
	ItemViewModelClass = UItemViewModel::StaticClass();
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
