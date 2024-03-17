#pragma once
#include <FW_GrowingArray.h>
#include <FW_Vector2.h>

#include "EntityHandle.h"

class Entity;
class EntityManager;

namespace Slush
{
	class PhysicsWorld;
}

class NPCWave
{
public:
	NPCWave(EntityManager& aEntityManager, Slush::PhysicsWorld& aPhysicsWorld);

	void Update();

	void StartWave(int aNumberOfNPCs);
	void SetPlayerHandle(const EntityHandle& aHandle);

private:
	void CreateNPC(const Vector2f& aPosition);
	bool IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance);

	EntityHandle myPlayerHandle;
	EntityManager& myEntityManager;
	Slush::PhysicsWorld& myPhysicsWorld;

	FW_GrowingArray<EntityHandle> myNPCs;

	float myPlayerClearanceRadius = 350.f;
	float myNPCClearanceRadius = 100.f;
};