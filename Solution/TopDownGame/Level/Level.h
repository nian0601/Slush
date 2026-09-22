#pragma once

#include <Core\Time.h>
#include <EntitySystem\EntityManager.h>

#include "LevelData.h"

class Level
{
public:
	Level(LevelData& aLevelData);

	Navmesh& GetNavmesh();
	NavmeshData& GetNavmeshDataAsset();
	LevelData& GetLevelDataAsset() { return *myLevelData; }
	Slush::EntityManager& GetEntityManager() { return myEntityManager; }

	void Update();
	void Render();

	void SpawnEnemyNormal();
	void SpawnEnemyFast();
	void SpawnEnemySlow();
	void DamageAllEnemies();

	int GetCurrentWaveIndex() const { return myCurrentWaveIndex; }
	int GetTotalWaveCount() const { return myLevelData->myTotalWaveCount; }

private:
	void SpawnEnemy(const char* aPrefabName);
	void SpawnTower(const char* aPrefabName);

	void UpdateWaveSpawning();
	void SpawnWaveEnemy();

	LevelData* myLevelData = nullptr;
	Slush::EntityManager myEntityManager;

	int myCurrentWaveIndex = 0;
	int myEnemiesRemainingToSpawnThisWave = 0;
	Slush::Timer myNextWaveTimer;
	Slush::Timer mySpawnStaggerTimer;
	FW_GrowingArray<Slush::EntityHandle> myActiveWaveEnemyHandles;
};
