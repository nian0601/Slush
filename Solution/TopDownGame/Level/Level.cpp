#include "stdafx.h"

#include "Level.h"
#include <Core\Engine.h>
#include <Core\Input.h>

#include "TopDownGameGlobals.h"
#include "WaveScaling.h"
#include "Components/HealthComponent.h"
#include "Components/BuildCostComponent.h"

#include <EntitySystem\EntityPrefab.h>
#include <EntitySystem\Components\SpriteComponent.h>

Level::Level(LevelData& aLevelData, bool aShouldCopyNavmesh)
{
	myLevelData = &aLevelData;
	FW_ASSERT(myLevelData->myNavmeshData.Get() != nullptr, "Level's LevelData has an unresolved NavmeshData reference");
	if (aShouldCopyNavmesh)
	{
		myNavmeshCopy = new Navmesh();
		myNavmeshCopy->CopyFrom(myLevelData->myNavmeshData.Get()->myNavmesh);
	}
	myResources = myLevelData->myStartingResources;
	SLUSH_INFO("[Resources] Starting balance: %d", myResources);
	myLives = myLevelData->myStartingLives;
	SLUSH_INFO("[Lives] Starting lives: %d", myLives);

	if (myLevelData->myPreFirstWaveGraceDuration > 0.f)
	{
		myNextWaveTimer.Start(myLevelData->myPreFirstWaveGraceDuration);
		myIsInGracePeriod = true;
		SLUSH_INFO("[Waves] Grace start: %.3f seconds (next %d, countdown %d)", GetSecondsUntilNextWave(), HasNextWave(), IsCountdownActive());
	}
	else if (myCurrentWaveIndex < myLevelData->myTotalWaveCount)
	{
		myEnemiesRemainingToSpawnThisWave = WaveScaling::GetEnemyCountForWave(myCurrentWaveIndex,
			myLevelData->myBaseEnemyCount, myLevelData->myEnemyCountPerWaveIncrement, myLevelData->myMaxEnemyCountPerWave);
	}

}

Level::~Level()
{
	delete myNavmeshCopy;
}

bool Level::HasNextWave() const
{
	return IsInGracePeriod() || myCurrentWaveIndex + 1 < myLevelData->myTotalWaveCount;
}

bool Level::IsCountdownActive() const
{
	return HasNextWave() && myNextWaveTimer.IsStarted() && !myNextWaveTimer.HasExpired();
}

float Level::GetSecondsUntilNextWave() const
{
	return IsCountdownActive() ? myNextWaveTimer.GetTimeRemaining() : 0.f;
}

void Level::AddResources(int anAmount)
{
	FW_ASSERT(anAmount >= 0, "Resources cannot be added with a negative amount");
	myResources += anAmount;
}

bool Level::TrySpendResources(int anAmount)
{
	FW_ASSERT(anAmount >= 0, "Resources cannot be spent with a negative amount");

	if (myResources < anAmount)
		return false;

	myResources -= anAmount;
	return true;
}

void Level::OnEnemyReachedGoal(const Slush::EntityPrefab& anEnemyPrefab)
{
	// Extra leaks arriving once lives are already gone (e.g. several on the same frame) are dropped silently.
	if (myLives <= 0)
		return;

	--myLives;
	SLUSH_INFO("[Lives] -1 from %s, now %d", anEnemyPrefab.GetAssetName().GetBuffer(), myLives);
}

PlaceTowerResult Level::TryPlaceTower(const Slush::EntityPrefab& aTowerPrefab, const Vector2f& aPosition)
{
	const Slush::SpriteComponent::Data& spriteData = aTowerPrefab.GetComponentData<Slush::SpriteComponent>();
	const Vector2f halfSize = spriteData.mySize * 0.5f;

	FW_GrowingArray<Vector2f> footprint;
	footprint.Add(aPosition - halfSize);
	footprint.Add(Vector2f{ aPosition.x + halfSize.x, aPosition.y - halfSize.y });
	footprint.Add(aPosition + halfSize);
	footprint.Add(Vector2f{ aPosition.x - halfSize.x, aPosition.y + halfSize.y });

	Navmesh& navmesh = GetNavmesh();
	if (!navmesh.IsAreaFullyOnMesh(footprint))
	{
		SLUSH_WARNING("[Level] Tower placement rejected: footprint is not fully on open navmesh");
		return PlaceTowerResult::FootprintBlocked;
	}

	const int cost = BuildCostComponent::GetCost(aTowerPrefab);
	if (!TrySpendResources(cost))
	{
		SLUSH_WARNING("[Level] Tower placement rejected: not enough resources (need %d, have %d)", cost, myResources);
		return PlaceTowerResult::NotAffordable;
	}

	SLUSH_INFO("[Resources] -%d for %s, now %d", cost, aTowerPrefab.GetAssetName().GetBuffer(), myResources);

	myEntityManager.CreateEntity(aPosition, aTowerPrefab);
	navmesh.CutHole(footprint);
	return PlaceTowerResult::Placed;
}

Navmesh& Level::GetNavmesh()
{
	return myNavmeshCopy ? *myNavmeshCopy : myLevelData->myNavmeshData.Get()->myNavmesh;
}

NavmeshData& Level::GetNavmeshDataAsset()
{
	return *myLevelData->myNavmeshData.Get();
}

void Level::Update()
{
	TopDownGameGlobals::GetInstance().SetLevel(this);

	Slush::Engine& engine = Slush::Engine::GetInstance();
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
	myEntityManager.Update();
	myEntityManager.EndFrame();
	UpdateResult();
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

void Level::UpdateWaveSpawning()
{
	if (myIsInGracePeriod)
	{
		if (!myNextWaveTimer.HasExpired())
			return;

		myNextWaveTimer = Slush::Timer();
		myIsInGracePeriod = false;
		if (myCurrentWaveIndex < myLevelData->myTotalWaveCount)
		{
			myEnemiesRemainingToSpawnThisWave = WaveScaling::GetEnemyCountForWave(myCurrentWaveIndex,
				myLevelData->myBaseEnemyCount, myLevelData->myEnemyCountPerWaveIncrement, myLevelData->myMaxEnemyCountPerWave);
		}
		SLUSH_INFO("[Waves] Grace end: wave 0 ready to spawn");
	}

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

		if (myEnemiesRemainingToSpawnThisWave == WaveScaling::GetEnemyCountForWave(myCurrentWaveIndex,
			myLevelData->myBaseEnemyCount, myLevelData->myEnemyCountPerWaveIncrement, myLevelData->myMaxEnemyCountPerWave))
			SLUSH_INFO("[Waves] Wave %d %s (next %d, countdown %d, remaining %.3f)", myCurrentWaveIndex, HasNextWave() ? "fighting" : "last wave", HasNextWave(), IsCountdownActive(), GetSecondsUntilNextWave());

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
			SLUSH_INFO("[Waves] Rest countdown start: %.3f seconds before wave %d (next %d, countdown %d)", GetSecondsUntilNextWave(), myCurrentWaveIndex + 1, HasNextWave(), IsCountdownActive());
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

void Level::UpdateResult()
{
	// Latched - once resolved, a late leak can't flip a win into a loss.
	if (myResult != LevelResult::InProgress)
		return;

	// Loss is checked first so it wins if both trigger on the same frame.
	if (myLives <= 0)
	{
		myResult = LevelResult::Lost;
	}
	else
	{
		// Only tracked wave enemies block a win - debug-spawned enemies don't.
		const bool allWavesSpawned = myCurrentWaveIndex >= myLevelData->myTotalWaveCount && myEnemiesRemainingToSpawnThisWave == 0;
		bool anyWaveEnemyAlive = false;
		for (const Slush::EntityHandle& handle : myActiveWaveEnemyHandles)
		{
			Slush::Entity* entity = handle.Get();
			if (entity && !entity->myIsMarkedForRemoval)
			{
				anyWaveEnemyAlive = true;
				break;
			}
		}

		if (allWavesSpawned && !anyWaveEnemyAlive)
			myResult = LevelResult::Won;
	}

	if (myResult != LevelResult::InProgress)
		SLUSH_INFO("[Level] Result: %s (lives %d)", myResult == LevelResult::Won ? "Won" : "Lost", myLives);
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
