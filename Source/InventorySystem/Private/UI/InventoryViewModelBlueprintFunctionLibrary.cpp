// Copyright Soccertitan 2025


#include "UI/InventoryViewModelBlueprintFunctionLibrary.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "Engine/AssetManager.h"
#include "Item/Fragment/ItemFragment_UI.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"

UItemInstanceViewModel* UInventoryViewModelBlueprintFunctionLibrary::CreateItemInstanceViewModel(UObject* Owner, const FItemInstance& ItemInstance)
{
	if (Owner)
	{
		UItemInstanceViewModel* ItemInstanceViewModel = NewObject<UItemInstanceViewModel>(Owner);
		ItemInstanceViewModel->SetItemInstance(ItemInstance);
		return ItemInstanceViewModel;
	}
	return nullptr;
}

bool UInventoryViewModelBlueprintFunctionLibrary::DoesItemHaveUIFragment(const TInstancedStruct<FItem>& Item)
{
	FAssetData AssetData;
	bool Result;
	FPrimaryAssetId AssetId = Item.Get<FItem>().GetItemDefinition()->GetPrimaryAssetId();
	UAssetManager::Get().GetPrimaryAssetData(AssetId, AssetData);
	AssetData.GetTagValue("ItemFragment_UI", Result);
	return Result;
}
