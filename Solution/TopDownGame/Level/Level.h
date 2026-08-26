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

private:
	LevelData* myLevelData = nullptr;
	Slush::EntityManager myEntityManager;

	int myCurrentWaveIndex = 0;
	Slush::Timer myNextWaveTimer;
};
