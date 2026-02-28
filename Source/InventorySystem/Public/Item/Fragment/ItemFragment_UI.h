// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemDefinition.h"
#include "ItemFragment_UI.generated.h"

class UItemInstanceViewModel;

/**
 * Describes information that is shown to a user.
 */
USTRUCT(BlueprintType)
struct FItemFragment_UI : public FItemFragment
{
	GENERATED_BODY()

	FItemFragment_UI();

	/** User facing description of the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI", meta = (MultiLine))
	FText Description;

	/** The user facing icon of the item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> Icon;

	/** The ItemInstanceViewModel to create. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI", meta = (AssetBundles = "ViewModel"), NoClear)
	TSoftClassPtr<UItemInstanceViewModel> ItemInstanceViewModelClass;

	/** A specialized widget to display additional item information. The class must implement the ItemViewModelInterface. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI", meta = (AssetBundles = "UI", MustImplement = "/Script/InventorySystem.ItemViewModelInterface"))
	TSoftClassPtr<UUserWidget> WidgetClass;

	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;
};
