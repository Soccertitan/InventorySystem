// Copyright Soccertitan 2025


#include "ItemContainer/ItemContainer.h"

#include "InventoryBlueprintFunctionLibrary.h"
#include "Item/ItemDefinition.h"
#include "InventoryGameplayTags.h"
#include "InventoryManagerComponent.h"
#include "ItemContainer/Rules/ItemContainerRule_AddItem.h"
#include "ItemContainer/Rules/ItemContainerRule_ItemQuantity.h"
#include "ItemContainer/Rules/ItemContainerRule_RemoveItem.h"
#include "Item/Fragment/ItemFragment_QuantityLimit.h"
#include "Net/UnrealNetwork.h"
#include "UI/ViewModel/ItemContainerViewModel.h"

UItemContainer::UItemContainer()
{
	ItemContainerViewModelClass = UItemContainerViewModel::StaticClass();
}

void UItemContainer::PostInitProperties()
{
	UObject::PostInitProperties();
	ItemInstanceContainer.Owner = this;
	InventoryManagerComponent = Cast<UInventoryManagerComponent>(GetOuter());
}

void UItemContainer::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemInstanceContainer, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CapacityLimit, Params);
	
	Params.Condition = COND_InitialOnly;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemContainerTag, Params);
}

#if UE_WITH_IRIS
void UItemContainer::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context,
	UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif

const FGameplayTagContainer& UItemContainer::GetOwnedTags() const
{
	return OwnedTags;
}

int32 UItemContainer::SetMaxCapacity(int32 NewCapacity, bool bOverride)
{
	if (HasAuthority())
	{
		const int32 OldLimit = CapacityLimit.GetMaxQuantity();
		CapacityLimit.SetLimitQuantity(true);
		if (bOverride && NewCapacity >= 0)
		{
			CapacityLimit.SetMaxQuantity(NewCapacity);
		}
		else
		{
			int32 AdjustedMaxCapacity = FMath::Max(0, CapacityLimit.GetMaxQuantity() + NewCapacity);
			CapacityLimit.SetMaxQuantity(AdjustedMaxCapacity);
		}
		
		if (OldLimit != CapacityLimit.GetMaxQuantity())
		{
			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CapacityLimit, this);
		}
	}
	return CapacityLimit.GetMaxQuantity();
}

int32 UItemContainer::GetMaxCapacity() const
{
	return CapacityLimit.GetMaxQuantity();
}

int32 UItemContainer::GetConsumedCapacity() const
{
	return ItemInstanceContainer.GetNum();
}

int32 UItemContainer::GetRemainingCapacity() const
{
	return GetMaxCapacity() - GetConsumedCapacity();
}

bool UItemContainer::IsAtMaxCapacity() const
{
	return GetConsumedCapacity() >= GetMaxCapacity();
}

const TArray<FItemInstance>& UItemContainer::GetItems() const
{
	return ItemInstanceContainer.GetItems();
}

FItemInstance* UItemContainer::FindItemByGuid(FGuid Guid) const
{
	return ItemInstanceContainer.GetItem(Guid);
}

FItemInstance UItemContainer::K2_FindItemByGuid(FGuid Guid) const
{
	if (FItemInstance* ItemInstance = FindItemByGuid(Guid))
	{
		return *ItemInstance;
	}
	return FItemInstance();
}

FItemInstance* UItemContainer::FindItemByDefinition(const UItemDefinition* ItemDefinition) const
{
	if (ItemDefinition)
	{
		for (const FItemInstance& ItemInstance : ItemInstanceContainer.GetItems())
		{
			if (ItemInstance.Item.GetPtr<FItem>()->GetItemDefinition() == ItemDefinition)
			{
				return const_cast<FItemInstance*>(&ItemInstance);
			}
		}
	}
	return nullptr;
}

FItemInstance UItemContainer::K2_FindItemByDefinition(const UItemDefinition* ItemDefinition) const
{
	if (FItemInstance* ItemInstance = FindItemByDefinition(ItemDefinition))
	{
		return *ItemInstance;
	}
	return FItemInstance();
}

TArray<FItemInstance*> UItemContainer::FindItemsByDefinition(const UItemDefinition* ItemDefinition) const
{
	TArray<FItemInstance*> Items;

	if (ItemDefinition)
	{
		for (const FItemInstance& ItemInstance : ItemInstanceContainer.GetItems())
		{
			if (ItemInstance.Item.GetPtr<FItem>()->GetItemDefinition() == ItemDefinition)
			{
				Items.Add(const_cast<FItemInstance*>(&ItemInstance));
			}
		}
	}
	return Items;
}

TArray<FItemInstance> UItemContainer::K2_FindItemsByDefinition(const UItemDefinition* ItemDefinition) const
{
	TArray<FItemInstance> Items;

	if (ItemDefinition)
	{
		for (const FItemInstance& ItemInstance : ItemInstanceContainer.GetItems())
		{
			if (ItemInstance.Item.GetPtr<FItem>()->GetItemDefinition() == ItemDefinition)
			{
				Items.Add(ItemInstance);
			}
		}
	}
	return Items;
}

FItemInstance* UItemContainer::FindMatchingItem(const TInstancedStruct<FItem>& Item) const
{
	if (Item.IsValid())
	{
		for (const FItemInstance& ItemInstance : ItemInstanceContainer.GetItems())
		{
			if (ItemInstance.Item.GetPtr<FItem>()->IsMatching(Item))
			{
				return const_cast<FItemInstance*>(&ItemInstance);
			}
		}
	}
	return nullptr;
}

FItemInstance UItemContainer::K2_FindMatchingItem( const TInstancedStruct<FItem>& Item) const
{
	if (FItemInstance* ItemInstance = FindMatchingItem(Item))
	{
		return *ItemInstance;
	}
	return FItemInstance();
}

TArray<FItemInstance*> UItemContainer::FindMatchingItems(const TInstancedStruct<FItem>& Item) const
{
	TArray<FItemInstance*> Result;
	if (Item.IsValid())
	{
		for (const FItemInstance& ItemInstance : ItemInstanceContainer.GetItems())
		{
			if (ItemInstance.Item.GetPtr<FItem>()->IsMatching(Item))
			{
				Result.Add(const_cast<FItemInstance*>(&ItemInstance));
			}
		}
	}
	return Result;
}

TArray<FItemInstance> UItemContainer::K2_FindMatchingItems(const TInstancedStruct<FItem>& Item) const
{
	TArray<FItemInstance> Result;
	if (Item.IsValid())
	{
		for (const FItemInstance& ItemInstance : ItemInstanceContainer.GetItems())
		{
			if (ItemInstance.Item.GetPtr<FItem>()->IsMatching(Item))
			{
				Result.Add(ItemInstance);
			}
		}
	}
	return Result;
}

bool UItemContainer::CanAddItem(const TInstancedStruct<FItem>& Item, FGameplayTag& OutError) const
{
	if (!Item.IsValid())
	{
		OutError = FInventoryGameplayTags::Get().ItemAddResult_Error_InvalidItem;
		return false;
	}

	if (Item.GetPtr<FItem>()->GetItemDefinition().IsNull())
	{
		OutError = FInventoryGameplayTags::Get().ItemAddResult_Error_InvalidItem;
		return false;
	}

	for (const TObjectPtr<UItemContainerRule_AddItem>& Rule : ItemContainerRules_AddItem)
	{
		if (Rule && !Rule->CanAddItem(Item, OutError))
		{
			return false;
		}
	}

	return true;
}

bool UItemContainer::CanRemoveItem(const TInstancedStruct<FItem>& Item) const
{
	if (!Item.IsValid())
	{
		return false;
	}

	for (const TObjectPtr<UItemContainerRule_RemoveItem>& Rule : ItemContainerRules_RemoveItem)
	{
		if (Rule && !Rule->CanRemoveItem(Item))
		{
			return false;
		}
	}

	return true;
}

bool UItemContainer::CanSplitItemStack(const FItemInstance& ItemInstance, const int32 Quantity) const
{
	if (!ItemInstance.IsValid())
	{
		return false;
	}

	if (ItemInstance.GetQuantity() <= 1)
	{
		return false;
	}

	if (Quantity >= ItemInstance.GetQuantity())
	{
		return false;
	}

	if (IsAtMaxCapacity())
	{
		return false;
	}

	if (IsItemAtStackQuantityLimit(ItemInstance.GetItem()))
	{
		return false;
	}

	return true;
}

bool UItemContainer::CanStackItems(const FItemInstance& TargetItemInstance,
	const FItemInstance& SourceItemInstance, const int32 QuantityToStack, int32& OutQuantity) const
{
	OutQuantity = 0;

	if (!SourceItemInstance.IsValid() || !TargetItemInstance.IsValid() || QuantityToStack <= 0)
	{
		return false;
	}

	if (!TargetItemInstance.GetItem().Get().IsMatching(SourceItemInstance.GetItem()))
	{
		return false;
	}

	const int32 TargetItemMaxQuantity = GetItemQuantityLimit(TargetItemInstance.GetItem());
	if (TargetItemMaxQuantity <= 1)
	{
		return false;
	}

	const int32 TargetItemAvailableSpace = TargetItemMaxQuantity - TargetItemInstance.GetQuantity();
	OutQuantity = FMath::Min(TargetItemAvailableSpace, QuantityToStack);
	OutQuantity = FMath::Min(OutQuantity, SourceItemInstance.GetQuantity());
	return OutQuantity > 0;
}

int32 UItemContainer::GetTotalItemQuantity(const TInstancedStruct<FItem>& Item) const
{
	if (!Item.IsValid())
	{
		return 0;
	}

	int32 Result = 0;

	for (const FItemInstance* Entry : FindItemsByDefinition(UInventoryBlueprintFunctionLibrary::GetItemDefinition(Item)))
	{
		Result = Result + Entry->GetQuantity();
	}
	
	return Result;
}

int32 UItemContainer::GetItemQuantityLimit(const TInstancedStruct<FItem>& Item) const
{
	if (Item.IsValid())
	{
		int32 Result = MAX_int32;
		for (const TObjectPtr<UItemContainerRule_ItemQuantity>& Rule : ItemContainerRules_ItemQuantity)
		{
			if (Rule)
			{
				const int32 ItemLimit = Rule->GetItemLimit(Item);
				Result = FMath::Min(Result, ItemLimit);
			}
		}

		if (const FItemFragment_QuantityLimit* Fragment =
			UInventoryBlueprintFunctionLibrary::GetItemDefinition(Item)->FindFragmentByType<FItemFragment_QuantityLimit>())
		{
			Result = FMath::Min(Result, Fragment->Item.GetMaxQuantity());
		}
		return FMath::Max(Result, 0);
	}
	return 0;
}

int32 UItemContainer::GetItemStackQuantityLimit(const TInstancedStruct<FItem>& Item) const
{
	if (Item.IsValid())
	{
		int32 Result = MAX_int32;
		for (const TObjectPtr<UItemContainerRule_ItemQuantity>& Rule : ItemContainerRules_ItemQuantity)
		{
			if (Rule)
			{
				const int32 ItemContainerLimit = Rule->GetItemContainerLimit(Item);
				const int32 InventoryManagerLimit = Rule->GetInventoryManagerLimit(Item);
				Result = FMath::Min(Result, ItemContainerLimit);
				Result = FMath::Min(Result, InventoryManagerLimit);
			}
		}

		if (const FItemFragment_QuantityLimit* Fragment =
			UInventoryBlueprintFunctionLibrary::GetItemDefinition(Item)->FindFragmentByType<FItemFragment_QuantityLimit>())
		{
			Result = FMath::Min(Result, Fragment->ItemContainer.GetMaxQuantity(), Fragment->InventoryManager.GetMaxQuantity());
		}
		return FMath::Max(Result, 0);
	}
	return 0;
}

int32 UItemContainer::GetRemainingItemStackCapacity(const TInstancedStruct<FItem>& Item) const
{
	const int32 StackQuantityLimit = FMath::Min(GetItemStackQuantityLimit(Item), GetRemainingCapacity());
	const int32 ItemStackCount = GetInventoryManagerComponent()->FindItemsByDefinition(UInventoryBlueprintFunctionLibrary::GetItemDefinition(Item)).Num();

	return FMath::Max(StackQuantityLimit - ItemStackCount, 0);
}

bool UItemContainer::IsItemAtStackQuantityLimit(const TInstancedStruct<FItem>& Item) const
{
	return GetRemainingItemStackCapacity(Item) <= 0;
}

void UItemContainer::MarkItemDirty(FItemInstance& ItemInstance)
{
	if (HasAuthority())
	{
		ItemInstanceContainer.MarkItemDirty(ItemInstance);
		Internal_OnItemChanged(ItemInstance);
		ItemInstance.PreReplicatedChangeItem = ItemInstance.Item;
	}
	else
	{
		// We are a client so we mark the array dirty to force rebuild.
		ItemInstanceContainer.MarkArrayDirty();
	}
}

void UItemContainer::Initialize()
{
	bOwnerIsNetAuthority = InventoryManagerComponent->HasAuthority();
}

void UItemContainer::GetAddItemPlan(const TInstancedStruct<FItem>& Item, FAddItemPlan& AddItemPlan) const
{
	//----------------------------------------------------------------------------------------------
	// 1. Determine how many quantities of this item we can add to the ItemContainer.
	//----------------------------------------------------------------------------------------------
	const int32 ItemQuantityLimit = GetItemQuantityLimit(Item);
	const int32 RemainingItemStackCapacity = GetRemainingItemStackCapacity(Item);
	int32 MaxItemQuantityToAdd = MAX_int32;
	if (!FMath::MultiplyAndCheckForOverflow(ItemQuantityLimit, RemainingItemStackCapacity, MaxItemQuantityToAdd))
	{
		MaxItemQuantityToAdd = MAX_int32;
	}
	MaxItemQuantityToAdd -= GetTotalItemQuantity(Item);
	int32 RemainingQuantityToAdd = FMath::Min(MaxItemQuantityToAdd, AddItemPlan.GetAmountToGive());

	//----------------------------------------------------------------------------------------------
	// 2. If auto stacking gather a list of all matching items in the list. We only try to auto
	// stack if the ItemStackMaxQuantity is greater than 1 and auto stacking is enabled.
	//----------------------------------------------------------------------------------------------
	TArray<FItemInstance*> MatchingItems = TArray<FItemInstance*>();
	if (bAutoStack && ItemQuantityLimit > 1)
	{
		MatchingItems = FindMatchingItems(Item);
	}
	
	//----------------------------------------------------------------------------------------------
	// 3. We add item quantities to existing items that are not at their max capacity for a single stack.
	//----------------------------------------------------------------------------------------------
	for (FItemInstance*& Match : MatchingItems)
	{
		if (RemainingQuantityToAdd <= 0)
		{
			break;
		}

		if (Match->GetQuantity() < ItemQuantityLimit)
		{
			const int32 QuantityToAdd = FMath::Min(RemainingQuantityToAdd, ItemQuantityLimit - Match->GetQuantity());
			RemainingQuantityToAdd = RemainingQuantityToAdd - QuantityToAdd;
			AddItemPlan.AddEntry(FAddItemPlanEntry(Match, QuantityToAdd));
		}
	}

	if (RemainingQuantityToAdd <= 0)
	{
		return;
	}

	//----------------------------------------------------------------------------------------------
	// 4. We add as many new stacks as allowed to the ItemContainer.
	//----------------------------------------------------------------------------------------------
	int32 NewStacksAdded = 0;
	while (RemainingQuantityToAdd > 0 && NewStacksAdded < RemainingItemStackCapacity)
	{
		const int32 QuantityToAdd = FMath::Min(RemainingQuantityToAdd, ItemQuantityLimit);
		TInstancedStruct<FItem> NewItem = Item;
		RemainingQuantityToAdd = RemainingQuantityToAdd - QuantityToAdd;
		AddItemPlan.AddEntry(FAddItemPlanEntry(NewItem, QuantityToAdd));
		NewStacksAdded++;
	}

	if (AddItemPlan.GetResult() == EAddItemResult::SomeItemsAdded)
	{
		AddItemPlan.Error = FInventoryGameplayTags::Get().ItemAddResult_Error_MaxStacksReached;
	}
}

void UItemContainer::Internal_OnItemAdded(const FItemInstance& ItemInstance)
{
	OnItemAdded(ItemInstance);
	K2_OnItemAdded(ItemInstance);
	OnItemAddedDelegate.Broadcast(ItemInstance);
	InventoryManagerComponent->OnItemAdded(ItemInstance);
}

void UItemContainer::Internal_OnItemRemoved(const FItemInstance& ItemInstance)
{
	OnItemRemoved(ItemInstance);
	K2_OnItemRemoved(ItemInstance);
	OnItemRemovedDelegate.Broadcast(ItemInstance);
	InventoryManagerComponent->OnItemRemoved(ItemInstance);
}

void UItemContainer::Internal_OnItemChanged(const FItemInstance& ItemInstance)
{
	OnItemChanged(ItemInstance);
	K2_OnItemChanged(ItemInstance);
	OnItemChangedDelegate.Broadcast(ItemInstance);
	InventoryManagerComponent->OnItemChanged(ItemInstance);
}

