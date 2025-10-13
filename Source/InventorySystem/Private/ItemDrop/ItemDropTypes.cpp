// Copyright Soccertitan


#include "ItemDrop/ItemDropTypes.h"

#include "ItemDrop/ItemDrop.h"


bool FItemDropParams::IsValid() const
{
	if (!ItemDropClass)
	{
		return false;
	}
	return true;
}
