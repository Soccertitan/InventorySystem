// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Engine/DataAsset.h"
#include "ItemSet.generated.h"

class UItemDefinition;

/**
 * Used internally by the UItemSet class. It holds an item that allows to manually edited.
 */
USTRUCT()
struct INVENTORYSYSTEM_API FItemSetInstance
{
	GENERATED_BODY()

	const TInstancedStruct<FItem>& GetItem() const;
	int32 GetQuantity() const;

	void PostSerialize(const FArchive& Ar);

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, meta = (StructTypeConst, FullyExpand=true, EditCondition=bShowItem,
		EditConditionHides, HideEditConditionToggle, AllowPrivateAccess, ShowOnlyInnerProperties))
	TInstancedStruct<FItem> Item;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, EditCondition=bShowItem,
		EditConditionHides, HideEditConditionToggle, AllowPrivateAccess))
	int32 Quantity = 1;
	
	UPROPERTY()
	bool bShowItem = false;

	/** Creates the item based on the ItemDefinition. Or clears it out when ItemDefinition is null. */
	void TryCreateItem();

#if WITH_EDITORONLY_DATA
	/** Hidden used as a TitleProperty in the UItemSet. */
	UPROPERTY(VisibleDefaultsOnly, meta = (EditCondition=false, EditConditionHides))
	FString EditorDisplayName;
#endif
	
	friend class UItemSet;
};
template<>
struct TStructOpsTypeTraits<FItemSetInstance> : public TStructOpsTypeTraitsBase2<FItemSetInstance>
{
	enum
	{
		WithPostSerialize = true,
   };
};

/**
 * Contains an array of items that can be designed in editor. Typically used as adding default startup items to an
 * InventoryContainer. If the ItemDefinition changes (i.e, Adding/Removing ItemDefFragments) after an instance is 'created'
 * you should delete the instance and recreate it to ensure the ItemDefFragments are added.
 */
UCLASS(const)
class INVENTORYSYSTEM_API UItemSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UItemSet();
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

	UPROPERTY(EditAnywhere, meta = (TitleProperty="EditorDisplayName"))
	TArray<FItemSetInstance> ItemInstances;
};
