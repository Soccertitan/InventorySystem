// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

INVENTORYSYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogInventorySystem, Log, All);

class FInventorySystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
