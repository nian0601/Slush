#pragma once

#include <Core\Time.h>

#include "LevelData.h"

class Level
{
public:
	Level();

	Navmesh& GetNavmesh();
	NavmeshData& GetNavmeshDataAsset();

	void Update();
	void Render();

private:
	LevelData* myLevelData = nullptr;

	int myCurrentWaveIndex = 0;
	Slush::Timer myNextWaveTimer;
};
