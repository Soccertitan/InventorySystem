// Copyright Soccertitan


#include "ItemDrop/ItemDropManager.h"

#include "ItemContainer/ItemContainer.h"
#include "InventoryManagerComponent.h"
#include "InventorySettings.h"
#include "ItemDrop/ItemDrop.h"
#include "ItemDrop/ItemDropItemContainer.h"
#include "Kismet/GameplayStatics.h"


AItemDropManager::AItemDropManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	InventoryManagerComponent = CreateDefaultSubobject<UInventoryManagerComponent>(TEXT("InventoryManagerComponent"));
}

void AItemDropManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InventoryManagerComponent->CreateItemContainer(UInventorySettings::GetDefaultItemContainerTag(), UItemDropItemContainer::StaticClass());
		InventoryManagerComponent->OnItemRemovedDelegate.AddUniqueDynamic(this, &AItemDropManager::OnItemRemoved);
	}
}

AItemDrop* AItemDropManager::TryCreateItemDrop(const TInstancedStruct<FItem>& Item, const FItemDropParams& Params)
{
	if (!HasAuthority() || !Params.IsValid())
	{
		return nullptr;
	}

	FAddItemPlanResult Result = InventoryManagerComponent->TryAddItemByGuid(Item, UInventorySettings::GetDefaultItemContainerTag());
	if (Result.ItemGuids.IsEmpty())
	{
		return nullptr;
	}

	return Internal_CreateItemDrop(Result.ItemGuids[0], Params);
}

AItemDrop* AItemDropManager::TryCreateItemDropFromItemInstance(const FItemInstance& ItemInstance,
	const FItemDropParams& Params, const int32 QuantityToDrop)
{
	if (!HasAuthority() || !Params.IsValid() || !ItemInstance.IsValid())
	{
		return nullptr;
	}

	TInstancedStruct<FItem> Item = ItemInstance.GetItemContainer()->GetInventoryManagerComponent()->
		ConsumeItemByGuid(ItemInstance.GetGuid(), QuantityToDrop);
	if (!Item.IsValid())
	{
		return nullptr;
	}

	FAddItemPlanResult Result = InventoryManagerComponent->TryAddItemByGuid(Item, UInventorySettings::GetDefaultItemContainerTag());
	if (Result.ItemGuids.IsEmpty())
	{
		return nullptr;
	}

	return Internal_CreateItemDrop(Result.ItemGuids[0], Params);
}

void AItemDropManager::RemoveItemDrop(AItemDrop* ItemDrop)
{
	if (IsValid(ItemDrop))
	{
		TInstancedStruct<FItem> Result = InventoryManagerComponent->TryRemoveItemByGuid(ItemDrop->ItemGuid);
		Internal_RemoveItemDrop(ItemDrop);
	}
}

void AItemDropManager::ClearItemDrops()
{
	while (ItemDrops.Num() >= MaxItemDrops)
	{
		if (IsValid(ItemDrops[0]))
		{
			RemoveItemDrop(ItemDrops[0]);
		}
	}
}

AItemDrop* AItemDropManager::Internal_CreateItemDrop(const FGuid ItemGuid, const FItemDropParams& Params)
{
	ClearItemDrops();

	FTransform Transform;
	Transform.SetLocation(Params.SpawnLocation);
	AItemDrop* NewItemDrop = GetWorld()->SpawnActorDeferred<AItemDrop>(
		Params.ItemDropClass,
		Transform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	NewItemDrop->ItemDropManager = this;
	NewItemDrop->InitializeItemDrop(ItemGuid, Params.Context);
	ItemDrops.Add(NewItemDrop);
	UGameplayStatics::FinishSpawningActor(NewItemDrop, Transform);
	return NewItemDrop;
}

void AItemDropManager::OnItemRemoved(UInventoryManagerComponent* InventoryManager, const FItemInstance& ItemInstance)
{
	for (TObjectPtr<AItemDrop>& ItemDrop : ItemDrops)
	{
		if (ItemDrop->ItemGuid == ItemInstance.GetGuid())
		{
			Internal_RemoveItemDrop(ItemDrop);
			return;
		}
	}
}

void AItemDropManager::Internal_RemoveItemDrop(AItemDrop* ItemDrop)
{
	ItemDrops.Remove(ItemDrop);
	ItemDrop->Destroy();
}
