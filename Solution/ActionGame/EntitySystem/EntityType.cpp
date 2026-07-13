#include "stdafx.h"

#include "EntityType.h"
#include <EntitySystem/Entity.h>

namespace
{
	const char* ourEntityTypeNames[] = { "Environment", "Player", "NPC", "Player Projectile", "NPC Projectile", "Pickup" };
	static_assert(IM_ARRAYSIZE(ourEntityTypeNames) == EntityType::ENTITYTYPE_COUNT);
}

bool IsPlayerOwned(const Slush::Entity& anEntity)
{
	return anEntity.myType == PLAYER || anEntity.myType == PLAYER_PROJECTILE;
}

bool IsNPCOwned(const Slush::Entity& anEntity)
{
	return anEntity.myType == NPC || anEntity.myType == NPC_PROJECTILE;
}

const char* ToString(EntityType aType)
{
	FW_ASSERT(aType >= 0 && aType < EntityType::ENTITYTYPE_COUNT, "Invalid EntityType");
	return ourEntityTypeNames[aType];
}

const char* const* GetEntityTypeNames()
{
	return ourEntityTypeNames;
}
