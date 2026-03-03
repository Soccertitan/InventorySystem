// Copyright Soccertitan 2025


#include "UI/InventoryViewModelBlueprintFunctionLibrary.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "InventoryFastTypes.h"
#include "Engine/AssetManager.h"
#include "Item/Fragment/ItemFragment_UI.h"
#include "UI/ViewModel/ItemInstanceViewModel.h"

UItemInstanceViewModel* UInventoryViewModelBlueprintFunctionLibrary::CreateItemInstanceViewModel(UObject* Owner, const FItemInstance& ItemInstance)
{
	if (Owner && ItemInstance.IsValid())
	{
		TSubclassOf<UItemInstanceViewModel> ItemInstanceViewModelClass = UItemInstanceViewModel::StaticClass();
		if (const FItemFragment_UI* UIFrag = UInventoryBlueprintFunctionLibrary::GetItemDefinition(ItemInstance.GetItem())->FindFragmentByType<FItemFragment_UI>())
		{
			if (!UIFrag->ItemInstanceViewModelClass.Get())
			{
				UAssetManager::Get().LoadAssetList({UIFrag->ItemInstanceViewModelClass.ToSoftObjectPath()})->WaitUntilComplete();
			}
			ItemInstanceViewModelClass = UIFrag->ItemInstanceViewModelClass.Get() ? UIFrag->ItemInstanceViewModelClass.Get() : UItemInstanceViewModel::StaticClass();
		}
		UItemInstanceViewModel* NewVM = NewObject<UItemInstanceViewModel>(Owner, ItemInstanceViewModelClass);
		NewVM->SetItemInstance(ItemInstance);
		return NewVM;
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
