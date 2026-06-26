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
#include "Serialization/MemoryReader.h"
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
	if (ItemContainerTag.IsValid())
	{
		return InventoryContainerInstanceContainer.GetItemContainerByTag(ItemContainerTag);
	}
	return nullptr;
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

TArray<UItemContainer*> UInventoryManagerComponent::GetItemContainers() const
{
	TArray<UItemContainer*> Result;
	Result.Empty(InventoryContainerInstanceContainer.GetItems().Num());
	
	for (const FItemContainerInstance& Entry : InventoryContainerInstanceContainer.GetItems())
	{
		Result.Add(Entry.GetItemContainer());
	}
	return Result;
}

void UInventoryManagerComponent::GetItems(TArray<FItemInstance*> OutItemInstances) const
{
	OutItemInstances.Empty();
	
	for (UItemContainer* ItemContainer : GetItemContainers())
	{
		TArray<FItemInstance*> ItemInstances;
		ItemContainer->GetItems(ItemInstances);
		OutItemInstances.Append(ItemInstances);
	}
}

TArray<FItemInstance> UInventoryManagerComponent::K2_GetItems() const
{
	TArray<FItemInstance> Result;
	for (UItemContainer* ItemContainer : GetItemContainers())
	{
		Result.Append(ItemContainer->K2_GetItems());
	}
	return Result;
}

FItemInstance* UInventoryManagerComponent::FindItem(const FItemInstanceHandle& Handle)
{
	if (Handle.IsValid())
	{
		UItemContainer* ItemContainer = Handle.GetItemContainer();
		if (FItemInstance* ItemInstance = ItemContainer->FindItemByGuid(Handle.GetGuid()))
		{
			return ItemInstance;
		}
	}
    return nullptr;
}

FItemInstance UInventoryManagerComponent::K2_FindItem(const FItemInstanceHandle& Handle)
{
	if (FItemInstance* ItemInstance = FindItem(Handle))
	{
		return *ItemInstance;
	}
	return FItemInstance();
}

void UInventoryManagerComponent::FindItemsByDefinition(const UItemDefinition* ItemDefinition, TArray<FItemInstance*>& Result) const
{
	Result.Empty();

	if (ItemDefinition)
	{
		for (const FItemContainerInstance& Entry : InventoryContainerInstanceContainer.GetItems())
		{
			TArray<FItemInstance*> ItemInstances;
			Entry.GetItemContainer()->FindItemsByDefinition(ItemDefinition, ItemInstances);
			Result.Append(ItemInstances);
		}
	}
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
	for (const TInstancedStruct<FItemDefinitionFragment>& Fragment : ItemDefinition->Fragments)
	{
		TInstancedStruct<FItemFragment> NewItemFragment = Fragment.Get<FItemDefinitionFragment>().GetItemFragment();
		if (NewItemFragment.IsValid())
		{
			ItemPtr->Fragments.Add(NewItemFragment);
		}
	}
	return Result;
}

FAddItemPlanResult UInventoryManagerComponent::AddItem(const TInstancedStruct<FItem>& Item, const int32 Quantity, UItemContainer* ItemContainer)
{
	FAddItemPlanResult Result;
	if (!HasAuthority())
	{
		Result.Error = FInventoryGameplayTags::Get().ItemAddResult_Error;
		return Result;
	}

	if (!ItemContainer || ItemContainer->GetInventoryManagerComponent() != this)
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
	ExecuteAddItemPlan(ItemContainer, AddItemPlan, Result.ItemInstanceHandles);
	Result.Result = AddItemPlan.GetResult();
	Result.AmountGiven = AddItemPlan.GetAmountGiven();
	Result.AmountToGive = AddItemPlan.GetAmountToGive();
	Result.Error = AddItemPlan.Error;
	return Result;
}

void UInventoryManagerComponent::RemoveItem(const FItemInstance* ItemInstance)
{
	if (!HasAuthority() || !ItemInstance || !ItemInstance->IsValid())
	{
		return;
	}

	if (ItemInstance->GetItemContainer()->CanRemoveItem(ItemInstance->Item))
	{
		RemoveItemInternal(ItemInstance->GetGuid(), ItemInstance->GetItemContainer());
	}
}

void UInventoryManagerComponent::K2_RemoveItem(const FItemInstanceHandle& Handle)
{
	if (const FItemInstance* ItemInstance = FindItem(Handle))
	{
		RemoveItem(ItemInstance);
	}
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
	ItemInstance->MarkItemDirty();

	if (NewQuantity <= 0)
	{
		RemoveItemInternal(ItemInstance->GetGuid(), ItemInstance->GetItemContainer());
	}

	return Delta;
}

int32 UInventoryManagerComponent::K2_ConsumeItem(const FItemInstanceHandle& Handle, const int32 QuantityToConsume)
{
	if (FItemInstance* ItemInstance = FindItem(Handle))
	{
		return ConsumeItem(ItemInstance, QuantityToConsume);
	}
	return 0;
}

int32 UInventoryManagerComponent::ConsumeItemsByDefinition(const UItemDefinition* ItemDefinition, UItemContainer* ItemContainer, int32 QuantityToConsume)
{
	int32 Result = 0;
	int32 QuantityRemaining = QuantityToConsume;
	
	TArray<FItemInstance*> ItemInstances;
	if (ItemContainer)
	{
		ItemContainer->FindItemsByDefinition(ItemDefinition, ItemInstances);
	}
	else
	{
		FindItemsByDefinition(ItemDefinition, ItemInstances);	
	}
	
	for (FItemInstance*& ItemInstance : ItemInstances)
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

void UInventoryManagerComponent::MoveItem(FItemInstance* ItemInstance, UItemContainer* ItemContainer, int32 QuantityToMove)
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
	ExecuteAddItemPlan_Move(ItemInstance, ItemContainer, AddItemPlan);
}

void UInventoryManagerComponent::K2_MoveItem(const FItemInstanceHandle Handle, UItemContainer* ItemContainer, int32 QuantityToMove)
{
	MoveItem(FindItem(Handle), ItemContainer, QuantityToMove);
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
	ItemInstance->MarkItemDirty();
	AddItemInternal(FGuid::NewGuid(), ItemInstance->Item, Quantity, ItemInstance->GetItemContainer());
}

void UInventoryManagerComponent::K2_SplitItemStack(const FItemInstanceHandle Handle, const int32 Quantity)
{
	SplitItemStack(FindItem(Handle), Quantity);
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
	TargetItemInstance->MarkItemDirty();
	SourceItemInstance->MarkItemDirty();
	if (SourceItemInstance->GetQuantity() <= 0)
	{
		RemoveItemInternal(SourceItemInstance->GetGuid(), SourceItemInstance->GetItemContainer());
	}
}

void UInventoryManagerComponent::K2_StackItems(const FItemInstanceHandle TargetHandle, const FItemInstanceHandle SourceHandle, const int32 Quantity)
{
	StackItems(FindItem(TargetHandle), FindItem(SourceHandle), Quantity);
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

	for (UItemContainer* ItemContainer : GetItemContainers())
	{
		SaveData.Add(ItemContainer);
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
				FPrimaryAssetId AssetId = UAssetManager::Get().GetPrimaryAssetIdForPath(ItemData.ItemDefinition.ToSoftObjectPath());
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
						AddItemInternal(ItemData.Guid, NewItem, ItemData.Quantity, NewContainer);
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
						AddItem(ItemInstance.GetItem(), ItemInstance.GetQuantity(), Container);	
					}
				}
			}
		}
	}
}

void UInventoryManagerComponent::ExecuteAddItemPlan(UItemContainer* ItemContainer,
	const FAddItemPlan& AddItemPlan, TArray<FItemInstanceHandle>& OutItemInstanceHandles)
{
	OutItemInstanceHandles.Empty();
	
	for (const FAddItemPlanEntry& Entry : AddItemPlan.GetEntries())
	{
		if (!Entry.IsValid())
		{
			continue;
		}
		
		if (Entry.ItemInstance)
		{
			Entry.ItemInstance->Quantity += Entry.QuantityToAdd;
			Entry.ItemInstance->MarkItemDirty();
			OutItemInstanceHandles.Add(*Entry.ItemInstance);
		}
		else
		{
			FGuid NewGuid = FGuid::NewGuid();
			OutItemInstanceHandles.Add(AddItemInternal(NewGuid, Entry.NewItem, Entry.QuantityToAdd, ItemContainer));
		}
	}
}

void UInventoryManagerComponent::ExecuteAddItemPlan_Move(FItemInstance* ItemInstance,
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
			Entry.ItemInstance->MarkItemDirty();
		}
		else
		{
			if (Entry.QuantityToAdd < ItemInstance->GetQuantity())
			{
				FGuid NewGuid = FGuid::NewGuid();
				AddItemInternal(NewGuid, Entry.NewItem, Entry.QuantityToAdd, ItemContainer);
				ItemInstance->Quantity -= Entry.QuantityToAdd;
			}
			else
			{
				AddItemInternal(ItemInstance->GetGuid(), Entry.NewItem, Entry.QuantityToAdd, ItemContainer);
				ItemInstance->WeakMovedToItemContainer = ItemContainer;
				RemoveItemInternal(ItemInstance->GetGuid(), ItemInstance->GetItemContainer());
				bItemWasMoved = true;
			}
		}
	}

	ItemInstance->MarkItemDirty();
	if (ItemInstance->GetQuantity() <= 0 && bItemWasMoved == false)
	{
		RemoveItemInternal(ItemInstance->GetGuid(), ItemInstance->GetItemContainer());
	}
}

FItemInstance UInventoryManagerComponent::AddItemInternal(const FGuid ItemGuid, const TInstancedStruct<FItem>& Item, const int32 Quantity, UItemContainer* ItemContainer)
{
	return ItemContainer->ItemInstanceContainer.AddItem(ItemGuid, Item, Quantity);
}

void UInventoryManagerComponent::RemoveItemInternal(FGuid ItemGuid, UItemContainer* ItemContainer)
{
	ItemContainer->ItemInstanceContainer.RemoveItem(ItemGuid);
}
