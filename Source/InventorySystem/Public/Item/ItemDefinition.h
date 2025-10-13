// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "UObject/AssetRegistryTagsContext.h"
#include "ItemDefinition.generated.h"


/**
 * A struct that represents custom static information in an ItemDefinition. Also initializes default values on the
 * created Item.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemFragment
{
	GENERATED_BODY()

	FItemFragment(){}
	virtual ~FItemFragment() {}

	/** Called when an item is created from an ItemDefinition. */
	virtual void SetDefaultValues(TInstancedStruct<FItem>& Item) const {}

	/**
	 * Called from the ItemDefinition when gathering the AssetTags for the AssetRegistrySearch functionality. Follow this
	 * design pattern to avoid clobbering AssetTag Names across different Fragments.
	 * For example if your ItemFragment is called "ItemFragment_Quantity" your RegistryTag Name should be
	 * "ItemFragment_Quantity_{Property}" Example code is below.
	 *
	 * @note 
	 * UObject::FAssetRegistryTag RegistryTag;
	 * RegistryTag.Type = UObject::FAssetRegistryTag::TT_Alphabetical;
	 * RegistryTag.Name = "ItemFragment_Quantity_{Property}";
	 * RegistryTag.Value = {Property in String Format};
	 * Context.AddTag(RegistryTag);
	 *
	 * @bug As of UE5.6 RegistryTags added this way will not show up in the Editor Window. But they are still being
	 * added successfully and can be queried.
	 */
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const {}

protected:

	/** Adds an ItemShard to the Item. */
	static void AddItemShard(TInstancedStruct<FItem>& Item, const TInstancedStruct<FItemShard>& Shard);
};

/**
 * Contains static information that describes an item.
 */
UCLASS(Const)
class INVENTORYSYSTEM_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UItemDefinition();
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Primary Asset")
	FPrimaryAssetType AssetType;

	/** The tags that this item has.
	 * @note You can search for items with specific tags through the AssetRegistry.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Definition")
	FGameplayTagContainer OwnedTags;

	/** The default stats for the item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Definition", meta = (ForceInlineRow, ClampMin=1))
	TMap<FGameplayTag, int32> DefaultStats;

	/**
	 * Defines custom item functionality.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Definition", meta = (FullyExpand=true, ExcludeBaseStruct))
	TArray<TInstancedStruct<FItemFragment>> Fragments;

	/** If set to false, this item should not be created. Useful for marking a deprecated item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Definition", AdvancedDisplay, AssetRegistrySearchable)
	bool bSpawnable = true;

	/** The struct that will be initialized for the InventoryItem */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Definition", AdvancedDisplay)
	TInstancedStruct<FItem> ItemClass;

	template<typename T> requires std::derived_from<T, FItemFragment>
	const T* FindFragmentByType() const;
};

/**
 * @return A const pointer to the first Fragment that matches the type.
 */
template <typename T> requires std::derived_from<T, FItemFragment>
const T* UItemDefinition::FindFragmentByType() const
{
	for (const TInstancedStruct<FItemFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			return FragmentPtr;
		}
	}
	return nullptr;
}