#pragma once

class Entity;

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

bool IsPlayerOwned(const Entity& anEntity);
bool IsNPCOwned(const Entity& anEntity);

const char* ToString(EntityType aType);
const char* const* GetEntityTypeNames();
