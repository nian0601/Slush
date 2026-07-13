#pragma once

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

const char* ToString(EntityType aType);
const char* const* GetEntityTypeNames();
