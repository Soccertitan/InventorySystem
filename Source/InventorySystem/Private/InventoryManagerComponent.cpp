// Copyright Soccertitan 2025


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

TInstancedStruct<FItem> UInventoryManagerComponent::CreateItem(const UItemDefinition* ItemDefinition)
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
	for (const TInstancedStruct<FItemFragment>& Fragment : ItemDefinition->Fragments)
	{
		Fragment.Get<FItemFragment>().SetDefaultValues(Result);
	}
	return Result;
}

FAddItemPlanResult UInventoryManagerComponent::TryAddItem(const TInstancedStruct<FItem>& Item, const int32 Quantity, UItemContainer* ItemContainer)
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
	
	if (Quantity <= 0)
	{
		Result.Error = FInventoryGameplayTags::Get().ItemAddResult_Error_InvalidQuantity;
		return Result;
	}

	if (!ItemContainer->CanAddItem(Item, Result.Error))
	{
		return Result;
	}

	FAddItemPlan AddItemPlan(Quantity);
	ItemContainer->GetAddItemPlan(Item, AddItemPlan);
	Result.ItemGuids = Internal_ExecuteAddItemPlan(ItemContainer, AddItemPlan);
	Result.Result = AddItemPlan.GetResult();
	Result.AmountGiven = AddItemPlan.GetAmountGiven();
	Result.AmountToGive = AddItemPlan.GetAmountToGive();
	Result.Error = AddItemPlan.Error;
	return Result;
}

FAddItemPlanResult UInventoryManagerComponent::TryAddItemByTag(const TInstancedStruct<FItem>& Item, const int32 Quantity, const FGameplayTag ItemContainerTag)
{
	return TryAddItem(Item, Quantity, FindItemContainerByTag(ItemContainerTag));
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

int32 UInventoryManagerComponent::ConsumeItem(FItemInstance* ItemInstance, const int32 QuantityToConsume)
{
	if (!HasAuthority() || !ItemInstance || !ItemInstance->IsValid() || QuantityToConsume <= 0)
	{
		return 0;
	}

	const int32 NewQuantity = FMath::Max(ItemInstance->Quantity - QuantityToConsume, 0);
	const int32 Delta = ItemInstance->Quantity - NewQuantity;

	ItemInstance->Quantity = NewQuantity;
	MarkItemDirty(*ItemInstance);

	if (NewQuantity <= 0)
	{
		Internal_RemoveItem(ItemInstance->GetGuid(), ItemInstance->GetItemContainer());
	}

	return Delta;
}

int32 UInventoryManagerComponent::ConsumeItemByGuid(const FGuid ItemGuid, const int32 QuantityToConsume)
{
	return ConsumeItem(FindItemByGuid(ItemGuid), QuantityToConsume);
}

int32 UInventoryManagerComponent::ConsumeItemsByDefinition(const UItemDefinition* ItemDefinition, int32 QuantityToConsume)
{
	int32 Result = 0;
	int32 QuantityRemaining = QuantityToConsume;
	for (FItemInstance*& ItemInstance : FindItemsByDefinition(ItemDefinition))
	{
		Result += ConsumeItem(ItemInstance, QuantityRemaining);
		QuantityRemaining = FMath::Max(QuantityToConsume - Result, 0);

		if (QuantityRemaining == 0)
		{
			return Result;
		}
	}
	return Result;
}

int32 UInventoryManagerComponent::ConsumeItemsByDefinitionInContainer(const UItemDefinition* ItemDefinition, const int32 QuantityToConsume, UItemContainer* ItemContainer)
{
	int32 Result = 0;
	if (ItemContainer)
	{
		int32 QuantityRemaining = QuantityToConsume;
		for (FItemInstance*& ItemInstance : ItemContainer->FindItemsByDefinition(ItemDefinition))
		{
			Result += ConsumeItem(ItemInstance, QuantityRemaining);
			QuantityRemaining = FMath::Max(QuantityToConsume - Result, 0);

			if (QuantityRemaining == 0)
			{
				return Result;
			}
		}
	}
	return Result;
}

void UInventoryManagerComponent::TryMoveItem(FItemInstance* ItemInstance, UItemContainer* ItemContainer, int32 QuantityToMove)
{
	if (!HasAuthority() || !ItemInstance || !ItemInstance->IsValid() || !ItemContainer || QuantityToMove <= 0)
	{
		return;
	}

	if (ItemInstance->GetItemContainer() == ItemContainer)
	{
		return;
	}

	FGameplayTag AddItemError;
	if (!ItemContainer->CanAddItem(ItemInstance->Item, AddItemError))
	{
		return;
	}

	QuantityToMove = FMath::Min(ItemInstance->GetQuantity(), QuantityToMove);
	TInstancedStruct<FItem> ItemToMove = ItemInstance->Item;
	FAddItemPlan AddItemPlan(QuantityToMove);
	ItemContainer->GetAddItemPlan(ItemToMove, AddItemPlan);
	Internal_ExecuteAddItemPlan_Move(ItemInstance, ItemContainer, AddItemPlan);
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

	if (!ItemInstance->GetItemContainer()->CanSplitItemStack(*ItemInstance, Quantity))
	{
		return;
	}

	ItemInstance->Quantity -= Quantity;
	MarkItemDirty(*ItemInstance);
	Internal_AddItem(FGuid::NewGuid(), ItemInstance->Item, Quantity, ItemInstance->GetItemContainer());
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
	if (!TargetItemInstance->GetItemContainer()->CanStackItems(*TargetItemInstance, *SourceItemInstance,
		Quantity, QuantityToStack))
	{
		return;
	}

	if (QuantityToStack <= 0)
	{
		return;
	}

	TargetItemInstance->Quantity += QuantityToStack;
	SourceItemInstance->Quantity -= QuantityToStack;
	MarkItemDirty(*TargetItemInstance);
	MarkItemDirty(*SourceItemInstance);
	if (SourceItemInstance->GetQuantity() <= 0)
	{
		Internal_RemoveItem(SourceItemInstance->GetGuid(), SourceItemInstance->GetItemContainer());
	}
}

void UInventoryManagerComponent::K2_StackItems(FGuid TargetItemGuid, FGuid SourceItemGuid, const int32 Quantity)
{
	StackItems(FindItemByGuid(TargetItemGuid), FindItemByGuid(SourceItemGuid), Quantity);
}

UItemContainer* UInventoryManagerComponent::CreateItemContainer(FGameplayTag ItemContainerTag, TSubclassOf<UItemContainer> ItemContainerClass)
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
	NewContainer->ItemContainerTag = ItemContainerTag;
	MARK_PROPERTY_DIRTY_FROM_NAME(UItemContainer, ItemContainerTag, NewContainer);
	NewContainer->Initialize();
	AddReplicatedSubObject(NewContainer, NewContainer->NetCondition);
	InventoryContainerInstanceContainer.AddItemContainer(NewContainer);
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
						Internal_AddItem(ItemData.Guid, NewItem, ItemData.Quantity, NewContainer);
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
	OnItemAddedDelegate.Broadcast(ItemInstance);
}

void UInventoryManagerComponent::OnItemRemoved(const FItemInstance& ItemInstance)
{
	OnItemRemovedDelegate.Broadcast(ItemInstance);
}

void UInventoryManagerComponent::OnItemChanged(const FItemInstance& ItemInstance)
{
	OnItemChangedDelegate.Broadcast(ItemInstance);
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
						TryAddItem(ItemInstance.GetItem(), ItemInstance.GetQuantity(), Container);	
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
			Entry.ItemInstance->Quantity += Entry.QuantityToAdd;
			MarkItemDirty(*Entry.ItemInstance);
			Result.Add(Entry.ItemInstance->GetGuid());
		}
		else
		{
			FGuid NewGuid = FGuid::NewGuid();
			Internal_AddItem(NewGuid, Entry.NewItem, Entry.QuantityToAdd, ItemContainer);
			Result.Add(NewGuid);
		}
	}

	return Result;
}

void UInventoryManagerComponent::Internal_ExecuteAddItemPlan_Move(FItemInstance* ItemInstance,
	UItemContainer* ItemContainer, const FAddItemPlan& AddItemPlan)
{
	bool bItemWasMoved = false;
	for (const FAddItemPlanEntry& Entry : AddItemPlan.GetEntries())
	{
		if (!Entry.IsValid())
		{
			continue;
		}
		
		if (Entry.ItemInstance)
		{
			Entry.ItemInstance->Quantity += Entry.QuantityToAdd;
			ItemInstance->Quantity -= Entry.QuantityToAdd;
			MarkItemDirty(*Entry.ItemInstance);
		}
		else
		{
			if (Entry.QuantityToAdd < ItemInstance->GetQuantity())
			{
				FGuid NewGuid = FGuid::NewGuid();
				Internal_AddItem(NewGuid, Entry.NewItem, Entry.QuantityToAdd, ItemContainer);
				ItemInstance->Quantity -= Entry.QuantityToAdd;
			}
			else
			{
				Internal_AddItem(ItemInstance->GetGuid(), Entry.NewItem, Entry.QuantityToAdd, ItemContainer);
				ItemInstance->WeakMovedToItemContainer = ItemContainer;
				Internal_RemoveItem(ItemInstance->GetGuid(), ItemInstance->GetItemContainer());
				bItemWasMoved = true;
			}
		}
	}

	MarkItemDirty(*ItemInstance);
	if (ItemInstance->GetQuantity() <= 0 && bItemWasMoved == false)
	{
		Internal_RemoveItem(ItemInstance->GetGuid(), ItemInstance->GetItemContainer());
	}
}

void UInventoryManagerComponent::Internal_AddItem(const FGuid ItemGuid, const TInstancedStruct<FItem>& Item, const int32 Quantity, UItemContainer* ItemContainer)
{
	ItemContainer->ItemInstanceContainer.AddItem(ItemGuid, Item, Quantity);
}

void UInventoryManagerComponent::Internal_RemoveItem(FGuid ItemGuid, UItemContainer* ItemContainer)
{
	ItemContainer->ItemInstanceContainer.RemoveItem(ItemGuid);
}
