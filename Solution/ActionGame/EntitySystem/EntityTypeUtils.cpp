#include "stdafx.h"

#include "EntityTypeUtils.h"
#include "Entity.h"

bool IsPlayerOwned(const Entity& anEntity)
{
	return anEntity.myType == PLAYER || anEntity.myType == PLAYER_PROJECTILE;
}

bool IsNPCOwned(const Entity& anEntity)
{
	return anEntity.myType == NPC || anEntity.myType == NPC_PROJECTILE;
}
