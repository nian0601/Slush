#include "NPCWave.h"

#include "Entity.h"
#include "EntityManager.h"
#include "EntityPrefab.h"
#include "NPCControllerComponent.h"
#include "ProjectileShootingComponent.h"

#include <FW_Math.h>

NPCWave::NPCWave(EntityManager& aEntityManager, Slush::PhysicsWorld& aPhysicsWorld)
	: myEntityManager(aEntityManager)
	, myPhysicsWorld(aPhysicsWorld)
{
}

void NPCWave::Update()
{
	for (int i = 0; i < myNPCs.Count();)
	{
		if (!myNPCs[i].IsValid())
		{
			myNPCs.RemoveCyclicAtIndex(i);
		}
		else
		{
			++i;
			continue;
		}
	}

	if (myNPCs.IsEmpty())
	{
		StartWave(FW_RandInt(2, 5));
	}
}

void NPCWave::StartWave(int aNumberOfNPCs)
{
	FW_GrowingArray<EntityHandle> playerHandles;
	myEntityManager.FindEntitiesOfType(Entity::PLAYER, playerHandles);

	if (playerHandles.IsEmpty())
	{
		SLUSH_ERROR("No players found, wont spawn any NPCs.");
		return;
	}

	if (playerHandles.Count() > 1)
		SLUSH_WARNING("More than one Player found, only first one will be used to determine NPC SpawnArea");

	Entity* player = playerHandles[0].Get();
	if (!player)
	{
		SLUSH_ERROR("Invalid Player found, wont spawn any NPCs");
		return;
	}

	const int iterationLimit = 100;
	int iterations = 0;
	Vector2f spawnArea = { 1280.f, 720.f };
	while (myNPCs.Count() < aNumberOfNPCs && iterations <= iterationLimit)
	{
		float randX = FW_RandFloat(-spawnArea.x, spawnArea.x);
		float randY = FW_RandFloat(-spawnArea.y, spawnArea.y);
		Vector2f position = player->myPosition;
		position.x += randX;
		position.y += randY;

		if (IsTooClose(player->myPosition, position, myPlayerClearanceRadius))
			continue;

		for (const EntityHandle& npc : myNPCs)
		{
			if (IsTooClose(npc.Get()->myPosition, position, myNPCClearanceRadius))
				continue;
		}

		CreateNPC(position);
	}
}

void NPCWave::CreateNPC(const Vector2f& aPosition)
{
	Entity* npc = myEntityManager.CreateEntity(aPosition, "NPC");
	if (npc->myProjectileShootingComponent)
		npc->myProjectileShootingComponent->TriggerCooldown();

	myNPCs.Add(npc->myHandle);
}

bool NPCWave::IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance)
{
	float distance2 = Length2(aPosition - aTestPosition);
	return distance2 <= FW_Square(aTestClearance);
}