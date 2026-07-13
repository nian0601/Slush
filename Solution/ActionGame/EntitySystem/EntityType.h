#pragma once

namespace Slush
{
	class Entity;
}

enum EntityType
{
	ENVIRONMENT,
	PLAYER,
	NPC,
	PLAYER_PROJECTILE,
	NPC_PROJECTILE,
	PICKUP,
	ENTITYTYPE_COUNT,
};

bool IsPlayerOwned(const Slush::Entity& anEntity);
bool IsNPCOwned(const Slush::Entity& anEntity);

const char* ToString(EntityType aType);
const char* const* GetEntityTypeNames();
