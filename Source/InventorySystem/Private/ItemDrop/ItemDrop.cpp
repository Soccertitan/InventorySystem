// Copyright Soccertitan 2025


#include "ItemDrop/ItemDrop.h"

#include "BlueprintNodeHelpers.h"
#include "Item/Item.h"
#include "ItemContainer/ItemContainer.h"
#include "InventoryManagerComponent.h"
#include "ItemDrop/ItemDropManager.h"
#include "Net/UnrealNetwork.h"


AItemDrop::AItemDrop()
{
	bHasCanTakeItem = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_CanTakeItem"), *this, *StaticClass());
	
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetNetUpdateFrequency(1.f);
}

void AItemDrop::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(AItemDrop, ItemGuid, Params);
}

void AItemDrop::TryTakeItem(UInventoryManagerComponent* InventoryManagerComponent, UItemContainer* Container)
{
	if (!HasAuthority() || !CanTakeItem(InventoryManagerComponent) || !Container)
	{
		return;
	}

	if (Container->GetInventoryManagerComponent() == ItemDropManager->InventoryManagerComponent)
	{
		return;
	}

	FItemInstance ItemInstance = ItemDropManager->InventoryManagerComponent->K2_FindItemByGuid(ItemGuid);
	FAddItemPlanResult Result = InventoryManagerComponent->TryAddItem(ItemInstance.GetItem(), ItemInstance.GetQuantity(), Container);

	ItemDropManager->InventoryManagerComponent->ConsumeItemByGuid(ItemGuid, Result.AmountGiven);
}

bool AItemDrop::CanTakeItem(UInventoryManagerComponent* InventoryManagerComponent) const
{
	if (!IsValid(InventoryManagerComponent))
	{
		return false;
	}

	if (InventoryManagerComponent == ItemDropManager->InventoryManagerComponent)
	{
		return false;
	}

	if (bHasCanTakeItem)
	{
		return K2_CanTakeItem(InventoryManagerComponent);
	}

	return true;
}

FItemInstance AItemDrop::GetItemInstance() const
{
	return ItemDropManager->InventoryManagerComponent->K2_FindItemByGuid(ItemGuid);
}

void AItemDrop::InitializeItemDrop(FGuid InItemGuid, UObject* Context)
{
	if (HasAuthority())
	{
		ItemGuid = InItemGuid;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemGuid, this);
		K2_InitializeItemDrop(ItemGuid, Context);
	}
}

