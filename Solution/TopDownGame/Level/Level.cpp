#include "stdafx.h"

#include "Level.h"
#include <Core\Assets\AssetStorage.h>
#include <Core\Engine.h>
#include <Core\Input.h>

#include "TopDownGameGlobals.h"
#include "WaveScaling.h"
#include "Components/HealthComponent.h"

#include <EntitySystem\EntityPrefab.h>

Level::Level()
{
	myLevelData = Slush::AssetRegistry::GetInstance().GetAsset<LevelData>("level_main");
	FW_ASSERT(myLevelData, "Level has no valid LevelData - expected a 'level_main' LevelData asset");
	FW_ASSERT(myLevelData->myNavmeshData.Get() != nullptr, "Level's LevelData has an unresolved NavmeshData reference");

	if (myCurrentWaveIndex < myLevelData->myTotalWaveCount)
	{
		myEnemiesRemainingToSpawnThisWave = WaveScaling::GetEnemyCountForWave(myCurrentWaveIndex,
			myLevelData->myBaseEnemyCount, myLevelData->myEnemyCountPerWaveIncrement, myLevelData->myMaxEnemyCountPerWave);
	}

	TopDownGameGlobals::GetInstance().SetLevel(this);
}

Level::~Level()
{
	TopDownGameGlobals::GetInstance().SetLevel(nullptr);
}

Navmesh& Level::GetNavmesh()
{
	return myLevelData->myNavmeshData.Get()->myNavmesh;
}

NavmeshData& Level::GetNavmeshDataAsset()
{
	return *myLevelData->myNavmeshData.Get();
}

void Level::Update()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	if (engine.GetInput().WasKeyReleased(Slush::Input::_1))
	{
		SpawnTower("Tower_Basic");
	}
	if (engine.GetInput().WasKeyReleased(Slush::Input::_2))
	{
		SpawnTower("Tower_ZeroDamage");
	}
	if (engine.GetInput().WasKeyReleased(Slush::Input::_3))
	{
		SpawnTower("Tower_Splash");
	}
	if (engine.GetInput().WasKeyReleased(Slush::Input::_7))
	{
		SpawnEnemyNormal();
	}
	if (engine.GetInput().WasKeyReleased(Slush::Input::_8))
	{
		DamageAllEnemies();
	}
	if (engine.GetInput().WasKeyReleased(Slush::Input::_9))
	{
		SpawnEnemyFast();
	}
	if (engine.GetInput().WasKeyReleased(Slush::Input::_0))
	{
		SpawnEnemySlow();
	}
	UpdateWaveSpawning();

	GetNavmesh().Update();
	myEntityManager.Update();
	myEntityManager.EndFrame();
}

void Level::Render()
{
	GetNavmesh().Render();
	myEntityManager.Render();
}

void Level::SpawnEnemyNormal()
{
	SpawnEnemy("Enemy_Normal");
}

void Level::SpawnEnemyFast()
{
	SpawnEnemy("Enemy_Fast");
}

void Level::SpawnEnemySlow()
{
	SpawnEnemy("Enemy_Slow");
}

void Level::SpawnEnemy(const char* aPrefabName)
{
	myEntityManager.CreateEntity(myLevelData->myStartPosition, aPrefabName);
}

void Level::SpawnTower(const char* aPrefabName)
{
	myEntityManager.CreateEntity(myLevelData->myStartPosition + Vector2f(200.f, 0.f), aPrefabName);
}

void Level::UpdateWaveSpawning()
{
	// Prune enemies that have died or reached the goal since last frame - both existing removal
	// paths (HealthComponent::DealDamage on kill, MovementComponent::Update on reaching the goal)
	// already mark the entity for removal, so there's no new removal logic needed here.
	for (int i = 0; i < myActiveWaveEnemyHandles.Count();)
	{
		Slush::Entity* entity = myActiveWaveEnemyHandles[i].Get();
		if (!entity || entity->myIsMarkedForRemoval)
			myActiveWaveEnemyHandles.RemoveCyclicAtIndex(i);
		else
			++i;
	}

	if (myEnemiesRemainingToSpawnThisWave > 0)
	{
		if (mySpawnStaggerTimer.IsStarted() && !mySpawnStaggerTimer.HasExpired())
			return;

		SpawnWaveEnemy();
		--myEnemiesRemainingToSpawnThisWave;
		mySpawnStaggerTimer.Start(myLevelData->myEnemySpawnInterval);
		return;
	}

	if (!myActiveWaveEnemyHandles.IsEmpty())
		return;

	if (myCurrentWaveIndex >= myLevelData->myTotalWaveCount)
		return;

	// The final wave clearing has no next wave to rest before, so it skips the timer
	// entirely rather than spending a full rest duration before myCurrentWaveIndex
	// reaches myTotalWaveCount and callers (e.g. the win/loss system) see "exhausted".
	bool isLastWave = (myCurrentWaveIndex == myLevelData->myTotalWaveCount - 1);
	if (!isLastWave)
	{
		if (!myNextWaveTimer.IsStarted())
		{
			myNextWaveTimer.Start(myLevelData->myInterWaveRestDuration);
			return;
		}

		if (!myNextWaveTimer.HasExpired())
			return;

		// Timer has no explicit reset - reassigning a fresh instance is how it's un-started, so the
		// next wave-cleared cycle waits out a full rest duration again instead of seeing a stale expiry.
		myNextWaveTimer = Slush::Timer();
	}

	++myCurrentWaveIndex;
	if (myCurrentWaveIndex < myLevelData->myTotalWaveCount)
	{
		myEnemiesRemainingToSpawnThisWave = WaveScaling::GetEnemyCountForWave(myCurrentWaveIndex,
			myLevelData->myBaseEnemyCount, myLevelData->myEnemyCountPerWaveIncrement, myLevelData->myMaxEnemyCountPerWave);
	}
}

void Level::SpawnWaveEnemy()
{
	float difficultyWeight = WaveScaling::GetDifficultyWeight(myCurrentWaveIndex, myLevelData->myDifficultyRampWaveCount);
	int unlockIndex = WaveScaling::PickUnlockedEnemyIndex(myLevelData->myEnemyUnlocks, myCurrentWaveIndex, difficultyWeight, FW_RandFloat());
	if (unlockIndex < 0)
		return;

	Slush::EntityPrefab* prefab = myLevelData->myEnemyUnlocks[unlockIndex].myEnemyPrefab.Get();
	if (!prefab)
		return;

	Slush::Entity* enemy = myEntityManager.CreateEntity(myLevelData->myStartPosition, *prefab);
	myActiveWaveEnemyHandles.Add(enemy->myHandle);
}

void Level::DamageAllEnemies()
{
	FW_GrowingArray<Slush::EntityHandle> entities;
	myEntityManager.GetAllEntities(entities);

	for (const Slush::EntityHandle& handle : entities)
	{
		Slush::Entity* entity = handle.Get();
		if (!entity || entity->myIsMarkedForRemoval)
			continue;

		if (HealthComponent* health = entity->GetComponent<HealthComponent>())
			health->DealDamage(10);
	}
}
