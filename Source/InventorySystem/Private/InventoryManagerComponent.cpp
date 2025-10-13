// Copyright Soccertitan


#include "InventoryManagerComponent.h"

#include "Item/ItemDefinition.h"
#include "InventoryGameplayTags.h"
#include "InventorySystem.h"
#include "Item/ItemSet.h"
#include "InventorySettings.h"
#include "ItemContainer/ItemContainer.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"


UInventoryManagerComponent::UInventoryManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheIsNetSimulated();
	InitializeStartupItems();
}

void UInventoryManagerComponent::PreNetReceive()
{
	Super::PreNetReceive();
	
	CacheIsNetSimulated();
}

void UInventoryManagerComponent::OnRegister()
{
	Super::OnRegister();

	CacheIsNetSimulated();
	InventoryContainerInstanceContainer.Owner = this;
}

void UInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryContainerInstanceContainer, Params);
}

#if WITH_EDITOR

void UInventoryManagerComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UInventoryManagerComponent, StartupItems))
	{
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd)
		{
			StartupItems.FindAndRemoveChecked(FGameplayTag());
			const FGameplayTag DefaultTag = UInventorySettings::GetDefaultItemContainerTag();
			const FStartupItems StartupItem(UInventorySettings::GetDefaultItemContainerClass());
			if (!StartupItems.Contains(DefaultTag))
			{
				StartupItems.Add(DefaultTag, StartupItem);
			}
			else
			{
				StartupItems.Add(FGameplayTag(), StartupItem);
			}
		}
	}
}
#endif

UItemContainer* UInventoryManagerComponent::FindItemContainerByTag(FGameplayTag ItemContainerTag) const
{
	return InventoryContainerInstanceContainer.GetItemContainerByTag(ItemContainerTag);
}

TArray<UItemContainer*> UInventoryManagerComponent::FindItemContainersByOwnedTags(FGameplayTagContainer OwnedTags)
{
	TArray<UItemContainer*> Result;
	for (const FItemContainerInstance& Entry : InventoryContainerInstanceContainer.GetItems())
	{
		if (Entry.GetItemContainer()->GetOwnedTags().HasAll(OwnedTags))
		{
			Result.Add(Entry.GetItemContainer());
		}
	}
	return Result;
}

const TArray<FItemContainerInstance>& UInventoryManagerComponent::GetItemContainers() const
{
	return InventoryContainerInstanceContainer.GetItems();
}

TArray<FItemInstance> UInventoryManagerComponent::GetItems() const
{
	TArray<FItemInstance> Result;
	for (const FItemContainerInstance& Entry : InventoryContainerInstanceContainer.GetItems())
	{
		Result.Append(Entry.GetItemContainer()->GetItems());
	}
	return Result;
}

FItemInstance* UInventoryManagerComponent::FindItemByGuid(FGuid ItemGuid) const
{
	for (const FItemContainerInstance& Entry : InventoryContainerInstanceContainer.GetItems())
    {
    	if (FItemInstance* ItemInstance = Entry.GetItemContainer()->FindItemByGuid(ItemGuid))
    	{
    		return ItemInstance;
    	}
    }
    return nullptr;
}

FItemInstance UInventoryManagerComponent::K2_FindItemByGuid(FGuid Guid) const
{
	if (FItemInstance* ItemInstance = FindItemByGuid(Guid))
	{
		return *ItemInstance;
	}
	return FItemInstance();
}

TArray<FItemInstance*> UInventoryManagerComponent::FindItemsByDefinition(const UItemDefinition* ItemDefinition) const
{
	TArray<FItemInstance*> Result;

	if (ItemDefinition)
	{
		for (const FItemContainerInstance& Entry : InventoryContainerInstanceContainer.GetItems())
		{
			Result.Append(Entry.GetItemContainer()->FindItemsByDefinition(ItemDefinition));
		}
	}
	return Result;
	
}

TArray<FItemInstance> UInventoryManagerComponent::K2_FindItemsByDefinition(const UItemDefinition* ItemDefinition) const
{
	TArray<FItemInstance> Result;

	if (ItemDefinition)
	{
		for (const FItemContainerInstance& Entry : InventoryContainerInstanceContainer.GetItems())
		{
			Result.Append(Entry.GetItemContainer()->K2_FindItemsByDefinition(ItemDefinition));
		}
	}
	return Result;
}

TInstancedStruct<FItem> UInventoryManagerComponent::CreateItem(const UItemDefinition* ItemDefinition, const int32 Quantity)
{
	if (!ItemDefinition ||
		!ItemDefinition->ItemClass.GetScriptStruct())
	{
		return TInstancedStruct<FItem>();
	}

	TInstancedStruct<FItem> Result;
	Result.InitializeAsScriptStruct(ItemDefinition->ItemClass.GetScriptStruct());
	FItem* ItemPtr = Result.GetMutablePtr<FItem>();
	ItemPtr->Initialize(ItemDefinition);
	ItemPtr->Quantity = FMath::Max(1, Quantity);
	for (const TInstancedStruct<FItemFragment>& Fragment : ItemDefinition->Fragments)
	{
		Fragment.Get<FItemFragment>().SetDefaultValues(Result);
	}
	return Result;
}

TInstancedStruct<FItem> UInventoryManagerComponent::DuplicateItem(const TInstancedStruct<FItem>& Item, const int32 Quantity)
{
	TInstancedStruct<FItem> Result;
	if (!Item.IsValid())
	{
		return Result;
	}

	if (Item.Get<FItem>().GetItemDefinition().IsNull())
	{
		return Result;
	}

	Result = Item;
	Result.GetMutablePtr<FItem>()->Quantity = FMath::Max(1, Quantity);
	return Result;
}

FAddItemPlanResult UInventoryManagerComponent::TryAddItem(const TInstancedStruct<FItem>& Item, UItemContainer* ItemContainer)
{
	FAddItemPlanResult Result;
	if (!HasAuthority())
	{
		Result.Error = FInventoryGameplayTags::Get().ItemAddResult_Error;
		return Result;
	}

	if (!ItemContainer)
	{
		Result.Error = FInventoryGameplayTags::Get().ItemAddResult_Error_ContainerNotFound;
		return Result;
	}

	if (!ItemContainer->CanAddItem(Item, Result.Error))
	{
		return Result;
	}

	FAddItemPlan AddItemPlan(Item.Get().GetQuantity()); 
	ItemContainer->GetAddItemPlan(Item, AddItemPlan);
	Result.ItemGuids = Internal_ExecuteAddItemPlan(ItemContainer, AddItemPlan);
	
	return Result;
}

FAddItemPlanResult UInventoryManagerComponent::TryAddItemByGuid(const TInstancedStruct<FItem>& Item, const FGameplayTag ItemContainerTag)
{
	return TryAddItem(Item, FindItemContainerByTag(ItemContainerTag));
}

TInstancedStruct<FItem> UInventoryManagerComponent::TryRemoveItem(FItemInstance* ItemInstance)
{
	TInstancedStruct<FItem> Result;
	if (!HasAuthority() || !ItemInstance || !ItemInstance->IsValid())
	{
		return Result;
	}

	if (!ItemInstance->GetItemContainer()->CanRemoveItem(ItemInstance->Item))
	{
		return Result;
	}

	Result = ItemInstance->Item;
	Internal_RemoveItem(ItemInstance->GetGuid(), ItemInstance->GetItemContainer());
	return Result;
}

TInstancedStruct<FItem> UInventoryManagerComponent::TryRemoveItemByGuid(const FGuid ItemGuid)
{
	return TryRemoveItem(FindItemByGuid(ItemGuid));
}

TInstancedStruct<FItem> UInventoryManagerComponent::ConsumeItem(FItemInstance* ItemInstance, const int32 QuantityToConsume)
{
	TInstancedStruct<FItem> Result;

	if (!HasAuthority() || !ItemInstance || !ItemInstance->IsValid() || QuantityToConsume <= 0)
	{
		return Result;
	}

	FItem* MutableItem = ItemInstance->Item.GetMutablePtr<FItem>();
	const int32 NewQuantity = FMath::Max(MutableItem->Quantity - QuantityToConsume, 0);
	const int32 Delta = MutableItem->Quantity - NewQuantity;
	Result = DuplicateItem(ItemInstance->Item, Delta);

	MutableItem->Quantity = NewQuantity;
	MarkItemDirty(*ItemInstance);

	if (NewQuantity <= 0)
	{
		Internal_RemoveItem(ItemInstance->GetGuid(), ItemInstance->GetItemContainer());
	}

	return Result;
}

TInstancedStruct<FItem> UInventoryManagerComponent::ConsumeItemByGuid(const FGuid ItemGuid, const int32 QuantityToConsume)
{
	return ConsumeItem(FindItemByGuid(ItemGuid), QuantityToConsume);
}

TArray<TInstancedStruct<FItem>> UInventoryManagerComponent::ConsumeItemsByDefinition(const UItemDefinition* ItemDefinition, int32 QuantityToConsume)
{
	TArray<TInstancedStruct<FItem>> Result;
	int32 QuantityRemaining = QuantityToConsume;
	for (FItemInstance*& ItemInstance : FindItemsByDefinition(ItemDefinition))
	{
		TInstancedStruct<FItem> ConsumedItem = ConsumeItem(ItemInstance, QuantityRemaining);
		Result.Add(ConsumedItem);
		QuantityRemaining = FMath::Max(QuantityRemaining - ConsumedItem.Get().GetQuantity(), 0);

		if (QuantityRemaining == 0)
		{
			return Result;
		}
	}
	return Result;
}

TArray<TInstancedStruct<FItem>> UInventoryManagerComponent::ConsumeItemsByDefinitionInContainer(
	const UItemDefinition* ItemDefinition, int32 QuantityToConsume, UItemContainer* ItemContainer)
{
	TArray<TInstancedStruct<FItem>> Result;
	if (ItemContainer)
	{
		int32 QuantityRemaining = QuantityToConsume;
		for (FItemInstance*& ItemInstance : ItemContainer->FindItemsByDefinition(ItemDefinition))
		{
			TInstancedStruct<FItem> ConsumedItem = ConsumeItem(ItemInstance, QuantityRemaining);
			Result.Add(ConsumedItem);
			QuantityRemaining = FMath::Max(QuantityRemaining - ConsumedItem.Get().GetQuantity(), 0);

			if (QuantityRemaining == 0)
			{
				return Result;
			}
		}
	}
	return Result;
}

void UInventoryManagerComponent::TryMoveItem(FItemInstance* ItemInstance, UItemContainer* Container, int32 QuantityToMove)
{
	if (!HasAuthority() || !ItemInstance || !ItemInstance->IsValid() || !Container || QuantityToMove <= 0)
	{
		return;
	}

	if (ItemInstance->GetItemContainer() == Container)
	{
		return;
	}

	FGameplayTag AddItemError;
	if (!Container->CanAddItem(ItemInstance->Item, AddItemError))
	{
		return;
	}

	QuantityToMove = FMath::Min(ItemInstance->Item.Get().GetQuantity(), QuantityToMove);
	TInstancedStruct<FItem> ItemToMove = DuplicateItem(ItemInstance->Item, QuantityToMove);
	FAddItemPlan AddItemPlan(QuantityToMove);
	Container->GetAddItemPlan(ItemToMove, AddItemPlan);
	Internal_ExecuteAddItemPlan_Move(ItemInstance, Container, AddItemPlan);
}

void UInventoryManagerComponent::K2_TryMoveItem(FGuid ItemGuid, FGameplayTag ItemContainerTag, int32 QuantityToMove)
{
	TryMoveItem(FindItemByGuid(ItemGuid), FindItemContainerByTag(ItemContainerTag), QuantityToMove);
}

void UInventoryManagerComponent::SplitItemStack(FItemInstance* ItemInstance, int32 Quantity)
{
	if (!HasAuthority() || !ItemInstance || !ItemInstance->IsValid() || Quantity <= 0)
	{
		return;
	}

	if (!ItemInstance->GetItemContainer()->CanSplitItemStack(ItemInstance->Item, Quantity))
	{
		return;
	}

	ItemInstance->Item.GetMutablePtr<FItem>()->Quantity -= Quantity;
	MarkItemDirty(*ItemInstance);
	TInstancedStruct<FItem> NewItem = DuplicateItem(ItemInstance->Item, Quantity);
	Internal_AddItem(FGuid::NewGuid(), NewItem, ItemInstance->GetItemContainer());
}

void UInventoryManagerComponent::K2_SplitItemStack(const FGuid ItemGuid, const int32 Quantity)
{
	SplitItemStack(FindItemByGuid(ItemGuid), Quantity);
}

void UInventoryManagerComponent::StackItems(FItemInstance* TargetItemInstance, FItemInstance* SourceItemInstance,
	const int32 Quantity)
{
	if (!HasAuthority() || !TargetItemInstance || !SourceItemInstance || Quantity <= 0)
	{
		return;
	}

	int32 QuantityToStack = 0;
	if (!TargetItemInstance->GetItemContainer()->CanStackItems(TargetItemInstance->Item, SourceItemInstance->Item,
		Quantity, QuantityToStack))
	{
		return;
	}

	if (QuantityToStack <= 0)
	{
		return;
	}

	TargetItemInstance->Item.GetMutablePtr<FItem>()->Quantity += QuantityToStack;
	SourceItemInstance->Item.GetMutablePtr<FItem>()->Quantity -= QuantityToStack;
	MarkItemDirty(*TargetItemInstance);
	MarkItemDirty(*SourceItemInstance);
	if (SourceItemInstance->Item.Get().GetQuantity() <= 0)
	{
		Internal_RemoveItem(SourceItemInstance->GetGuid(), SourceItemInstance->GetItemContainer());
	}
}

void UInventoryManagerComponent::K2_StackItems(FGuid TargetItemGuid, FGuid SourceItemGuid, const int32 Quantity)
{
	StackItems(FindItemByGuid(TargetItemGuid), FindItemByGuid(SourceItemGuid), Quantity);
}

UItemContainer* UInventoryManagerComponent::CreateItemContainer(FGameplayTag ItemContainerTag,
	TSubclassOf<UItemContainer> ItemContainerClass)
{
	if (!HasAuthority() || !ItemContainerClass)
	{
		return nullptr;
	}

	if (!ItemContainerTag.IsValid())
	{
		return nullptr;
	}

	if (InventoryContainerInstanceContainer.GetItemContainerByTag(ItemContainerTag))
	{
		UE_LOG(LogInventorySystem, Verbose,
			   TEXT("CreateItemContainer already has %s as the ItemContainerTag. The ItemContainer will not be created."),
			   *ItemContainerTag.ToString());
		return nullptr;
	}

	UItemContainer* NewContainer = NewObject<UItemContainer>(this, ItemContainerClass);
	NewContainer->Initialize();
	AddReplicatedSubObject(NewContainer);
	InventoryContainerInstanceContainer.AddItemContainer(NewContainer, ItemContainerTag);
	return NewContainer;
}

void UInventoryManagerComponent::RemoveItemContainer(UItemContainer* ItemContainer)
{
	if (!HasAuthority() || ItemContainer->GetInventoryManagerComponent() != this)
	{
		return;
	}

	ItemContainer->ItemInstanceContainer.Reset();
	InventoryContainerInstanceContainer.RemoveItemContainer(ItemContainer);
	RemoveReplicatedSubObject(ItemContainer);
	ItemContainer->MarkAsGarbage();
}

TArray<FItemContainerSaveData> UInventoryManagerComponent::GetSaveData() const
{
	TArray<FItemContainerSaveData> SaveData;

	for (const FItemContainerInstance& Entry : GetItemContainers())
	{
		SaveData.Add(Entry);
	}

	return SaveData;
}

void UInventoryManagerComponent::LoadSavedData(const TArray<FItemContainerSaveData>& SaveData)
{
	if (!HasAuthority())
	{
		return;
	}

	FName bSpawnableName = GET_MEMBER_NAME_CHECKED(UItemDefinition, bSpawnable);
	
	//----------------------------------------------------------
	// 1. Remove all existing Items and Containers
	//----------------------------------------------------------
	TArray<UItemContainer*> ContainersToDestroy;
	for (const FItemContainerInstance& Container : InventoryContainerInstanceContainer.GetItems())
	{
		if (IsValid(Container.GetItemContainer()))
		{
			ContainersToDestroy.Add(Container.GetItemContainer());
		}
	}
	
	for (UItemContainer* Container : ContainersToDestroy)
	{
		RemoveItemContainer(Container);
	}
	
	//----------------------------------------------------------
	// 2. Restore Containers
	//----------------------------------------------------------
	for (const FItemContainerSaveData& ContainerData : SaveData)
	{
		if (!ContainerData.ContainerClass.Get())
		{
			UAssetManager::Get().LoadAssetList({ContainerData.ContainerClass.ToSoftObjectPath()})->WaitUntilComplete();
		}
		
		UItemContainer* NewContainer = CreateItemContainer(
			ContainerData.ContainerTag,
			ContainerData.ContainerClass.Get()
		);
		
		if (NewContainer)
		{
			// Serialize ItemContainer properties
			FMemoryReader MemoryReader(ContainerData.ByteData);
			FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
			Archive.ArIsSaveGame = true;
			NewContainer->Serialize(Archive);

			//----------------------------------------------------------
			// 3. Restore Items into their containers.
			//----------------------------------------------------------
			for (const FItemSaveData& ItemData : ContainerData.InventoryItemsSaveData)
			{
				if (ItemData.ItemDefinition.IsNull())
				{
					// Do not restore the item's data if the ItemDef is invalid.
					continue;
				}

				FAssetData AssetData;
				bool bSpawnable = true;
				FPrimaryAssetId AssetId = ItemData.ItemDefinition->GetPrimaryAssetId();
				UAssetManager::Get().GetPrimaryAssetData(AssetId, AssetData);
				AssetData.GetTagValue(bSpawnableName, bSpawnable);

				if (bSpawnable)
				{
					// Only restore the item if it's spawnable.
					if (!ItemData.ItemDefinition.Get())
					{
						UAssetManager::Get().LoadAssetList({ItemData.ItemDefinition.ToSoftObjectPath()})->WaitUntilComplete();
					}
				
					TInstancedStruct<FItem> NewItem;
					FMemoryReader MemReader(ItemData.ByteData);
					FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
					Ar.ArIsSaveGame = true;
					NewItem.Serialize(Ar);
					if (NewItem.IsValid())
					{
						Internal_AddItem(ItemData.Guid, NewItem, NewContainer);
					}
				}
			}
		}
	}
}

bool UInventoryManagerComponent::HasAuthority() const
{
	return !bCachedIsNetSimulated;
}

void UInventoryManagerComponent::MarkItemDirty(FItemInstance& ItemInstance)
{
	if (ItemInstance.GetItemContainer())
	{
		ItemInstance.GetItemContainer()->MarkItemDirty(ItemInstance);
	}
}

void UInventoryManagerComponent::OnContainerAdded(const FItemContainerInstance& ContainerInstance)
{
	OnContainerAddedDelegate.Broadcast(this, ContainerInstance.GetItemContainer());
}

void UInventoryManagerComponent::OnContainerRemoved(const FItemContainerInstance& ContainerInstance)
{
	OnContainerRemovedDelegate.Broadcast(this, ContainerInstance.GetItemContainer());
}

void UInventoryManagerComponent::OnItemAdded(const FItemInstance& ItemInstance)
{
	OnItemAddedDelegate.Broadcast(this, ItemInstance);
}

void UInventoryManagerComponent::OnItemRemoved(const FItemInstance& ItemInstance)
{
	OnItemRemovedDelegate.Broadcast(this, ItemInstance);
}

void UInventoryManagerComponent::OnItemChanged(const FItemInstance& ItemInstance)
{
	OnItemChangedDelegate.Broadcast(this, ItemInstance);
}

void UInventoryManagerComponent::CacheIsNetSimulated()
{
	bCachedIsNetSimulated = IsNetSimulating();
}

void UInventoryManagerComponent::InitializeStartupItems()
{
	if (!HasAuthority())
	{
		return;
	}

	for (const TTuple<FGameplayTag, FStartupItems>& Startup : StartupItems)
	{
		if (UItemContainer* Container = CreateItemContainer(Startup.Key, Startup.Value.ItemContainerClass))
		{
			for (const UItemSet* ItemSet : Startup.Value.ItemSets)
			{
				if (ItemSet)
				{
					for (const FItemSetInstance& ItemInstance : ItemSet->ItemInstances)
					{
						TryAddItem(ItemInstance.GetItem(), Container);	
					}
				}
			}
		}
	}
}

TArray<FGuid> UInventoryManagerComponent::Internal_ExecuteAddItemPlan(UItemContainer* ItemContainer,
	const FAddItemPlan& AddItemPlan)
{
	TArray<FGuid> Result;
	
	for (const FAddItemPlanEntry& Entry : AddItemPlan.GetEntries())
	{
		if (!Entry.IsValid())
		{
			continue;
		}
		
		if (Entry.ItemInstance)
		{
			Entry.ItemInstance->Item.GetMutable().Quantity += Entry.QuantityToAdd;
			MarkItemDirty(*Entry.ItemInstance);
			Result.Add(Entry.ItemInstance->GetGuid());
		}
		else
		{
			TInstancedStruct<FItem> NewItem = DuplicateItem(Entry.NewItem, Entry.QuantityToAdd);
			FGuid NewGuid = FGuid::NewGuid();
			Internal_AddItem(NewGuid, NewItem, ItemContainer);
			Result.Add(NewGuid);
		}
	}

	return Result;
}

void UInventoryManagerComponent::Internal_ExecuteAddItemPlan_Move(FItemInstance* ItemInstance,
	UItemContainer* ItemContainer, const FAddItemPlan& AddItemPlan)
{
	FItem* Item = ItemInstance->Item.GetMutablePtr<FItem>();
	bool bItemWasMoved = false;
	for (const FAddItemPlanEntry& Entry : AddItemPlan.GetEntries())
	{
		if (!Entry.IsValid())
		{
			continue;
		}
		
		if (Entry.ItemInstance)
		{
			Entry.ItemInstance->Item.GetMutable().Quantity += Entry.QuantityToAdd;
			Item->Quantity -= Entry.QuantityToAdd;
			MarkItemDirty(*Entry.ItemInstance);
		}
		else
		{
			TInstancedStruct<FItem> NewItem = DuplicateItem(Entry.NewItem, Entry.QuantityToAdd);
			if (Entry.QuantityToAdd < Item->GetQuantity())
			{
				FGuid NewGuid = FGuid::NewGuid();
				Internal_AddItem(NewGuid, NewItem, ItemContainer);
				Item->Quantity -= Entry.QuantityToAdd;
			}
			else
			{
				Internal_AddItem(ItemInstance->GetGuid(), NewItem, ItemContainer);
				Internal_RemoveItem(ItemInstance->GetGuid(), ItemInstance->GetItemContainer());
				bItemWasMoved = true;
			}
		}
	}

	MarkItemDirty(*ItemInstance);
	if (Item->GetQuantity() <= 0 && bItemWasMoved == false)
	{
		Internal_RemoveItem(ItemInstance->GetGuid(), ItemInstance->GetItemContainer());
	}
}

void UInventoryManagerComponent::Internal_AddItem(const FGuid ItemGuid, const TInstancedStruct<FItem>& Item, UItemContainer* ItemContainer)
{
	ItemContainer->ItemInstanceContainer.AddItem(ItemGuid, Item);
}

void UInventoryManagerComponent::Internal_RemoveItem(FGuid ItemGuid, UItemContainer* ItemContainer)
{
	ItemContainer->ItemInstanceContainer.RemoveItem(ItemGuid);
}
