// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "UObject/AssetRegistryTagsContext.h"
#include "ItemDefinition.generated.h"


class UItemInstanceComponentViewModel;
class UUserWidget;
class UItemInstanceViewModel;

/**
 * A struct that represents custom static information in an ItemDefinition.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemDefinitionFragment
{
	GENERATED_BODY()

	FItemDefinitionFragment(){}
	virtual ~FItemDefinitionFragment() {}

	/** Called when an item is created from an ItemDefinition. */
	virtual TInstancedStruct<FItemFragment> GetItemFragment() const { return TInstancedStruct<FItemFragment>(); }
	
	/** 
	 * Called on each fragment in the ItemDefinition when a new ItemDefinition is set. 
	 * @note If planning to expose via a UPROPERTY. Ensure the meta = (AssetBundles = "ViewModel") to have the 
	 * ItemInstanceViewModel have the loaded component ready on construction. */
	virtual TSubclassOf<UItemInstanceComponentViewModel> GetItemInstanceComponentViewModel() const { return nullptr; }

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
	
	/** User facing text of the item name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FText ItemName;
	
	/** User facing description of the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (MultiLine=true))
	FText Description;

	/** The user facing icon of the item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftObjectPtr<UTexture2D> Icon;
	
	/** The ItemInstanceViewModel to create. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AssetBundles = "ViewModel"), NoClear)
	TSoftClassPtr<UItemInstanceViewModel> ItemInstanceViewModelClass;
	
	/** A specialized widget to display additional item information. The class must implement the ItemViewModelInterface. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AssetBundles = "UI", MustImplement = "/Script/InventorySystem.ItemViewModelInterface"))
	TSoftClassPtr<UUserWidget> WidgetClass;

	/** 
	 * The tags that this item has.
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
	TArray<TInstancedStruct<FItemDefinitionFragment>> Fragments;

	/** If set to false, this item should not be created. Useful for marking a deprecated item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Definition", AdvancedDisplay, AssetRegistrySearchable)
	bool bSpawnable = true;

	/** The struct that will be initialized for the InventoryItem */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Definition", AdvancedDisplay)
	TInstancedStruct<FItem> ItemClass;

	template<typename T> requires std::derived_from<T, FItemDefinitionFragment>
	const T* FindFragmentByType() const;
};

/**
 * @return A const pointer to the first Fragment that matches the type.
 */
template <typename T> requires std::derived_from<T, FItemDefinitionFragment>
const T* UItemDefinition::FindFragmentByType() const
{
	for (const TInstancedStruct<FItemDefinitionFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			return FragmentPtr;
		}
	}
	return nullptr;
}