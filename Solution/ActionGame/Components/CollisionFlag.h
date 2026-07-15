#pragma once

#include "EntitySystem/Components/PhysicsComponent.h"

namespace CollisionUtils
{
	enum CollisionFlag
	{
		COL_ENVIRONMENT,
		COL_PLAYER,
		COL_NPC,
		COL_PLAYER_PROJECTILE,
		COL_NPC_PROJECTILE,
		COL_PICKUP,
		COLLISIONFLAG_COUNT,
	};

	const char* ToString(CollisionFlag aFlag);
	const char* const* GetNames();
	const char* const* GetSerializationNames();
}

struct GamePhysicsData : public Slush::PhysicsComponent::Data
{
	void OnParseCollisionFlags(Slush::AssetParser::Handle aComponentHandle) override;
	void OnBuildCollisionFlagsUI() override;
};
