#include "NPCWave.h"

#include "Entity.h"
#include "HealthComponent.h"
#include <FW_Math.h>
#include "ProjectileManager.h"
#include "SpriteComponent.h"
#include "ProjectileShootingComponent.h"
#include "NPCControllerComponent.h"
#include "CollisionComponent.h"
#include "EntityManager.h"

NPCWave::NPCWave(EntityManager& aEntityManager, ProjectileManager& aProjectileManager)
	: myEntityManager(aEntityManager)
	, myProjectileManager(aProjectileManager)
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
			myProjectileManager.CheckCollisionsWithEntity(*myNPCs[i].Get());

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
	Entity* entity = myEntityManager.CreateEntity();
	entity->myType = Entity::NPC;
	entity->myPosition = aPosition;
	entity->mySpriteComponent = new SpriteComponent(*entity);
	entity->mySpriteComponent->MakeCircle(20.f, 0xFF0000FF);
	entity->myProjectileShootingComponent = new ProjectileShootingComponent(*entity, myProjectileManager);
	entity->myProjectileShootingComponent->SetCooldown(1.f);
	entity->myProjectileShootingComponent->TriggerCooldown();
	entity->myNPCControllerComponent = new NPCControllerComponent(*entity);
	entity->myNPCControllerComponent->SetTarget(myPlayerHandle);
	entity->myCollisionComponent = new CollisionComponent(*entity);
	entity->myCollisionComponent->SetSize(20.f);
	entity->myHealthComponent = new HealthComponent(*entity);
	entity->myHealthComponent->SetMaxHealth(5);
	myNPCs.Add(entity->myHandle);
}

bool NPCWave::IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance)
{
	float distance2 = Length2(aPosition - aTestPosition);
	return distance2 <= FW_Square(aTestClearance);
}
