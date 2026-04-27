// Copyright Soccertitan 2025


#include "ItemDrop/ItemDrop.h"

#include "BlueprintNodeHelpers.h"
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
	DOREPLIFETIME_WITH_PARAMS_FAST(AItemDrop, ItemInstanceHandle, Params);
}

void AItemDrop::TakeItem(UInventoryManagerComponent* InventoryManagerComponent, UItemContainer* ItemContainer)
{
	if (!HasAuthority() || !CanTakeItem(InventoryManagerComponent) || !ItemContainer)
	{
		return;
	}

	if (ItemContainer->GetInventoryManagerComponent() == ItemDropManager->InventoryManagerComponent)
	{
		return;
	}

	FItemInstance ItemInstance = UInventoryManagerComponent::K2_FindItem(ItemInstanceHandle);
	FAddItemPlanResult Result = InventoryManagerComponent->AddItem(ItemInstance.GetItem(), ItemInstance.GetQuantity(), ItemContainer);

	ItemDropManager->InventoryManagerComponent->K2_ConsumeItem(ItemInstanceHandle, Result.AmountGiven);
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
	return UInventoryManagerComponent::K2_FindItem(ItemInstanceHandle);
}

void AItemDrop::InitializeItemDrop(FItemInstanceHandle Handle, UObject* Context)
{
	if (HasAuthority())
	{
		ItemInstanceHandle = Handle;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemInstanceHandle, this);
		K2_InitializeItemDrop(Handle, Context);
	}
}

