#include "NPCWave.h"

#include "Entity.h"
#include "EntityManager.h"
#include "EntityPrefab.h"
#include "NPCControllerComponent.h"
#include "ProjectileShootingComponent.h"

#include <FW_Math.h>

NPCWave::NPCWave(EntityManager& aEntityManager, ProjectileManager& aProjectileManager, Slush::PhysicsWorld& aPhysicsWorld)
	: myEntityManager(aEntityManager)
	, myProjectileManager(aProjectileManager)
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
	const int iterationLimit = 100;
	int iterations = 0;
	Vector2f spawnArea = { 1280.f, 720.f };
	while (myNPCs.Count() < aNumberOfNPCs && iterations <= iterationLimit)
	{
		Vector2f position = FW_RandomVector2f() * spawnArea;
		if(IsTooClose(myPlayerHandle.Get()->myPosition, position, myPlayerClearanceRadius))
			continue;

		for (const EntityHandle& npc : myNPCs)
		{
			if (IsTooClose(npc.Get()->myPosition, position, myNPCClearanceRadius))
				continue;
		}

		CreateNPC(position);
	}
}

void NPCWave::SetPlayerHandle(const EntityHandle& aHandle)
{
	myPlayerHandle = aHandle;

	for (const EntityHandle& npc : myNPCs)
		npc.Get()->myNPCControllerComponent->SetTarget(myPlayerHandle);
}

void NPCWave::CreateNPC(const Vector2f& aPosition)
{
	EntityPrefab prefab;
	prefab.myEntityType = Entity::NPC;
	prefab.mySprite.myEnabled = true;
	prefab.mySprite.myRadius = 20.f;
	prefab.mySprite.myColor = 0xFF0000FF;
	prefab.myAnimation.myEnabled = true;
	prefab.myProjectileShooting.myEnabled = true;
	prefab.myProjectileShooting.myCooldown = 1.f;
	prefab.myNPCController.myEnabled = true;
	prefab.myHealth.myEnabled = true;
	prefab.myHealth.myMaxHealth = 5;
	prefab.myPhysics.myEnabled = true;
	prefab.myPhysics.myMatchSprite = true;

	Entity* npc = myEntityManager.CreateEntity(aPosition, prefab, myPhysicsWorld, myProjectileManager);
	npc->myProjectileShootingComponent->TriggerCooldown();
	npc->myNPCControllerComponent->SetTarget(myPlayerHandle);
	myNPCs.Add(npc->myHandle);
}

bool NPCWave::IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance)
{
	float distance2 = Length2(aPosition - aTestPosition);
	return distance2 <= FW_Square(aTestClearance);
}