#pragma once

#include "EntityHandle.h"

class EntityManager;
class NPCWave;

namespace Slush
{
	class PhysicsWorld;
}

class Level
{
public:
	Level(EntityManager& aEntityManager, Slush::PhysicsWorld& aPhysicsWorld);
	~Level();

	void Update();

	bool IsPlayerDead() const;

private:
	Slush::PhysicsWorld& myPhysicsWorld;
	EntityManager& myEntityManager;
	EntityHandle myPlayerHandle;

	NPCWave* myNPCWave;
};
