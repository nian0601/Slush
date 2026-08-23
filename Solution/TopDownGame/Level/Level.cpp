#include "stdafx.h"

#include "Level.h"
#include <Core\Assets\AssetStorage.h>

Level::Level()
{
	myLevelData = Slush::AssetRegistry::GetInstance().GetAsset<LevelData>("level_main");
	FW_ASSERT(myLevelData, "Level has no valid LevelData - expected a 'level_main' LevelData asset");
	FW_ASSERT(myLevelData->myNavmeshData.Get() != nullptr, "Level's LevelData has an unresolved NavmeshData reference");
}

Navmesh& Level::GetNavmesh()
{
	return myLevelData->myNavmeshData.Get()->myNavmesh;
}

void Level::Update()
{
	GetNavmesh().Update();
}

void Level::Render()
{
	GetNavmesh().Render();
}
