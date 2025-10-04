#include "stdafx.h"

#include "ActionGameGlobals.h"
#include "EntitySystem/EntityManager.h"
#include "NPCWave.h"

#include "Components/NPCControllerComponent.h"
#include "Components/ProjectileShootingComponent.h"

NPCWave::NPCWave()
	: myEntityManager(ActionGameGlobals::GetInstance().GetEntityManager())
{
	myWaveTimer.Start(1.f);
}

void NPCWave::Update()
{
	if (ActionGameGlobals::GetInstance().myDebugSettings.myPauseEnemySpawning)
		return;

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

	if (myWaveTimer.HasExpired())
	{
		StartWave(FW_RandInt(2, 5));
		myWaveTimer.Start(5.f);
	}
}

void NPCWave::StartWave(int aNumberOfNPCs)
{
	FW_GrowingArray<EntityHandle> playerHandles;
	myEntityManager.FindEntitiesOfType(EntityType::PLAYER, playerHandles);

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

	++myWaveCounter;

	const int iterationLimit = 100;
	int iterations = 0;
	Vector2f spawnArea = { 1920.f, 1080.f };
	Vector2f spawnAreaMargin = { 75.f, 75.f };
	while (myNPCs.Count() < aNumberOfNPCs && iterations <= iterationLimit)
	{
		++iterations;

		float randX = FW_RandFloat(spawnAreaMargin.x, spawnArea.x - spawnAreaMargin.x);
		float randY = FW_RandFloat(spawnAreaMargin.y, spawnArea.y - spawnAreaMargin.y);
		Vector2f position;
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
	const char* npcPrefab = "NPC";
	if (myWaveCounter > 5 && FW_RandFloat() > 0.5f)
		npcPrefab = "NPCRanged";
	
	Entity* npc = myEntityManager.CreateEntity(aPosition, npcPrefab);
	if (ProjectileShootingComponent* projShoot = npc->GetComponent<ProjectileShootingComponent>())
		projShoot->TriggerCooldown();

	myNPCs.Add(npc->myHandle);
}

bool NPCWave::IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance)
{
	float distance2 = Length2(aPosition - aTestPosition);
	return distance2 <= FW_Square(aTestClearance);
}