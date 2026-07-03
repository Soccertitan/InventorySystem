// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "StructUtils/InstancedStruct.h"
#include "ItemInstanceComponentViewModel.generated.h"

class UItemDefinition;
struct FItem;
struct FItemInstance;

/**
 * Custom data that can be added to an ItemInstanceViewModel. Generally created by an ItemInstanceComponent.
 */
UCLASS(Abstract)
class INVENTORYSYSTEM_API UItemInstanceComponentViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	
protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnItemInstanceSet(const FItemInstance& ItemInstance);
	virtual void OnItemInstanceSet_Implementation(const FItemInstance& ItemInstance) {}
	
	UFUNCTION(BlueprintNativeEvent)
	void OnItemSet(const TInstancedStruct<FItem>& Item);
	virtual void OnItemSet_Implementation(const TInstancedStruct<FItem>& Item) {}

	UFUNCTION(BlueprintNativeEvent)
	void OnItemDefinitionSet(const UItemDefinition* ItemDefinition);
	virtual void OnItemDefinitionSet_Implementation(const UItemDefinition* ItemDefinition) {}
	
private:
	friend class UItemInstanceViewModel;
};
