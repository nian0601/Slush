#include "NPCWave.h"

#include "Entity.h"
#include "HealthComponent.h"
#include <FW_Math.h>
#include "ProjectileManager.h"
#include "SpriteComponent.h"
#include "ProjectileShootingComponent.h"
#include "NPCControllerComponent.h"
#include "CollisionComponent.h"

NPCWave::NPCWave(Entity& aPlayerEntity, ProjectileManager& aProjectileManager)
	: myPlayerEntity(aPlayerEntity)
	, myProjectileManager(aProjectileManager)
{
}

NPCWave::~NPCWave()
{
	myNPCs.DeleteAll();
}

void NPCWave::Update()
{
	for (int i = 0; i < myNPCs.Count();)
	{
		myNPCs[i]->Update();
		myProjectileManager.CheckCollisionsWithEntity(*myNPCs[i]);

		if (HealthComponent* health = myNPCs[i]->myHealthComponent)
		{
			if (health->IsDead())
			{
				myNPCs.DeleteCyclicAtIndex(i);
				continue;
			}
		}

		++i;
	}

	if (myNPCs.IsEmpty())
	{
		StartWave(FW_RandInt(2, 5));
	}
}

void NPCWave::Render()
{
	for (Entity* npc : myNPCs)
		npc->Render();
}

void NPCWave::StartWave(int aNumberOfNPCs)
{
	const int iterationLimit = 100;
	int iterations = 0;
	Vector2f spawnArea = { 1280.f, 720.f };
	while (myNPCs.Count() < aNumberOfNPCs && iterations <= iterationLimit)
	{
		Vector2f position = FW_RandomVector2f() * spawnArea;
		if(IsTooClose(myPlayerEntity.myPosition, position, myPlayerClearanceRadius))
			continue;

		for (Entity* npc : myNPCs)
		{
			if (IsTooClose(npc->myPosition, position, myNPCClearanceRadius))
				continue;
		}

		CreateNPC(position);
	}
}

void NPCWave::CreateNPC(const Vector2f& aPosition)
{
	Entity* entity = new Entity();
	entity->myType = Entity::NPC;
	entity->myPosition = aPosition;
	entity->mySpriteComponent = new SpriteComponent(*entity);
	entity->mySpriteComponent->MakeCircle(20.f, 0xFF0000FF);
	entity->myProjectileShootingComponent = new ProjectileShootingComponent(*entity, myProjectileManager);
	entity->myProjectileShootingComponent->SetCooldown(1.f);
	entity->myProjectileShootingComponent->TriggerCooldown();
	entity->myNPCControllerComponent = new NPCControllerComponent(*entity);
	entity->myNPCControllerComponent->SetTarget(myPlayerEntity);
	entity->myCollisionComponent = new CollisionComponent(*entity);
	entity->myCollisionComponent->SetSize(20.f);
	entity->myHealthComponent = new HealthComponent(*entity);
	entity->myHealthComponent->SetMaxHealth(5);
	myNPCs.Add(entity);
}

bool NPCWave::IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance)
{
	float distance2 = Length2(aPosition - aTestPosition);
	return distance2 <= FW_Square(aTestClearance);
}
