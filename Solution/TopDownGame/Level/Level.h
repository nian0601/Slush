#pragma once

#include <Core\Time.h>
#include <EntitySystem\EntityManager.h>

#include "LevelData.h"

class Level
{
public:
	Level();
	~Level();

	Navmesh& GetNavmesh();
	NavmeshData& GetNavmeshDataAsset();
	LevelData& GetLevelDataAsset() { return *myLevelData; }

	void Update();
	void Render();

	void SpawnEnemyNormal();
	void SpawnEnemyFast();
	void SpawnEnemySlow();
	void SpawnTowerBasic();
	void SpawnTowerZeroDamage();
	void DamageAllEnemies();

private:
	void SpawnEnemy(const char* aPrefabName);

	LevelData* myLevelData = nullptr;
	Slush::EntityManager myEntityManager;

	int myCurrentWaveIndex = 0;
	Slush::Timer myNextWaveTimer;
};
