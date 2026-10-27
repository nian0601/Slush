#pragma once

#include <Core\Time.h>
#include <EntitySystem\EntityManager.h>

#include "LevelData.h"

namespace Slush
{
	class EntityPrefab;
}

enum class PlaceTowerResult
{
	Placed,
	FootprintBlocked,
	TooCloseToEndpoint,
	NotAffordable,
};

namespace TowerPlacementUtils
{
	// True when the axis-aligned rect reaches closer than aRadius to aPoint; exactly touching is allowed.
	// Per point, so a level with several spawns or goals can loop over it.
	bool DoesRectOverlapCircle(const Vector2f& aRectMin, const Vector2f& aRectMax, const Vector2f& aPoint, float aRadius);
}

enum class LevelResult
{
	InProgress,
	Won,
	Lost,
};

class Level
{
public:
	// anIsEditMode is the Level Editor's still level: no wave spawning and no win/lose result, only
	// entities (debug-spawned enemies) update.
	Level(LevelData& aLevelData, bool aShouldCopyNavmesh, bool anIsEditMode = false);
	~Level();

	Navmesh& GetNavmesh();
	NavmeshData& GetNavmeshDataAsset();
	LevelData& GetLevelDataAsset() { return *myLevelData; }
	Slush::EntityManager& GetEntityManager() { return myEntityManager; }

	void Update();
	void Render();

	void SpawnEnemyNormal();
	void SpawnEnemyFast();
	void SpawnEnemySlow();
	void SpawnEnemy(const Slush::EntityPrefab& anEnemyPrefab);
	void DamageAllEnemies();
	void ClearEnemies();

	bool IsInGracePeriod() const { return myIsInGracePeriod; }
	bool HasNextWave() const;
	bool IsCountdownActive() const;
	float GetSecondsUntilNextWave() const;

	int GetCurrentWaveIndex() const { return myCurrentWaveIndex; }
	int GetTotalWaveCount() const { return myLevelData->myTotalWaveCount; }
	int GetResources() const { return myResources; }
	void AddResources(int anAmount);
	bool TrySpendResources(int anAmount);

	int GetLives() const { return myLives; }
	void OnEnemyReachedGoal(const Slush::EntityPrefab& anEnemyPrefab);

	LevelResult GetResult() const { return myResult; }

	Vector2f ResolveTowerPosition(const Slush::EntityPrefab& aTowerPrefab, const Vector2f& aRequestedPosition);
	PlaceTowerResult TryPlaceTower(const Slush::EntityPrefab& aTowerPrefab, const Vector2f& aPosition);

private:
	void SpawnEnemy(const char* aPrefabName);

	void UpdateWaveSpawning();
	void SpawnWaveEnemy();
	void UpdateResult();

	LevelData* myLevelData = nullptr;
	bool myIsEditMode = false;
	Navmesh* myNavmeshCopy = nullptr;
	Slush::EntityManager myEntityManager;
	int myResources = 0;
	int myLives = 0;
	LevelResult myResult = LevelResult::InProgress;

	int myCurrentWaveIndex = 0;
	int myEnemiesRemainingToSpawnThisWave = 0;
	bool myIsInGracePeriod = false;
	Slush::Timer myNextWaveTimer;
	Slush::Timer mySpawnStaggerTimer;
	FW_GrowingArray<Slush::EntityHandle> myActiveWaveEnemyHandles;
};
