// Copyright Soccertitan 2025


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
		InventoryManagerComponent->CreateItemContainer(UInventorySettings::GetItemContainerTag(), UItemDropItemContainer::StaticClass());
		InventoryManagerComponent->OnItemRemovedDelegate.AddUniqueDynamic(this, &AItemDropManager::OnItemRemoved);
	}
}

AItemDrop* AItemDropManager::CreateItemDrop(const TInstancedStruct<FItem>& Item, const int32 Quantity, const FItemDropParams& Params)
{
	if (!HasAuthority() || !Params.IsValid())
	{
		return nullptr;
	}
	
	UItemContainer* ItemContainer = InventoryManagerComponent->FindItemContainerByTag(UInventorySettings::GetItemContainerTag());
	FAddItemPlanResult Result = InventoryManagerComponent->AddItem(Item, Quantity, ItemContainer);
	if (Result.ItemInstanceHandles.IsEmpty())
	{
		return nullptr;
	}

	return CreateItemDropInternal(Result.ItemInstanceHandles[0], Params);
}

AItemDrop* AItemDropManager::CreateItemDropFromItemInstance(const FItemInstance& ItemInstance,
	const FItemDropParams& Params, const int32 QuantityToDrop)
{
	if (!HasAuthority() || !Params.IsValid() || !ItemInstance.IsValid())
	{
		return nullptr;
	}

	int32 Quantity = ItemInstance.GetInventoryManagerComponent()->
		K2_ConsumeItem(ItemInstance.GetHandle(), QuantityToDrop);
	if (Quantity <= 0)
	{
		return nullptr;
	}

	UItemContainer* ItemContainer = InventoryManagerComponent->FindItemContainerByTag(UInventorySettings::GetItemContainerTag());
	FAddItemPlanResult Result = InventoryManagerComponent->AddItem(ItemInstance.GetItem(), Quantity, ItemContainer);
	if (Result.ItemInstanceHandles.IsEmpty())
	{
		return nullptr;
	}

	return CreateItemDropInternal(Result.ItemInstanceHandles[0], Params);
}

void AItemDropManager::RemoveItemDrop(AItemDrop* ItemDrop)
{
	if (IsValid(ItemDrop))
	{
		InventoryManagerComponent->K2_RemoveItem(ItemDrop->ItemInstanceHandle);
		RemoveItemDropInternal(ItemDrop);
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

AItemDrop* AItemDropManager::CreateItemDropInternal(const FItemInstanceHandle& Handle, const FItemDropParams& Params)
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
	NewItemDrop->InitializeItemDrop(Handle, Params.Context);
	ItemDrops.Add(NewItemDrop);
	UGameplayStatics::FinishSpawningActor(NewItemDrop, Transform);
	return NewItemDrop;
}

void AItemDropManager::OnItemRemoved(const FItemInstance& ItemInstance)
{
	for (TObjectPtr<AItemDrop>& ItemDrop : ItemDrops)
	{
		if (ItemDrop->ItemInstanceHandle == ItemInstance.GetHandle())
		{
			RemoveItemDropInternal(ItemDrop);
			return;
		}
	}
}

void AItemDropManager::RemoveItemDropInternal(AItemDrop* ItemDrop)
{
	ItemDrops.Remove(ItemDrop);
	ItemDrop->Destroy();
}
