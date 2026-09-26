#pragma once

#include "StateStack/IGameState.h"

class Level;
class LevelData;
enum class PlaceTowerResult;

namespace Slush
{
	class EntityPrefab;
}

// Deterministic tower placement/attack test harness, started with the -towertest flag.
// F1-F5 run placement scenarios through Level::TryPlaceTower and log PASS/FAIL, F6/F7 spawn enemies.
class TowerTestState : public Slush::IGameState
{
public:
	TowerTestState(LevelData& aLevelData);
	~TowerTestState();

	GameStateResult Update() override;
	void Render() override;

private:
	void UpdateScenarioKeys();
	void RunPlacementScenario(const char* aName, const Slush::EntityPrefab& aTowerPrefab, const Vector2f& aPosition, PlaceTowerResult anExpectedResult);

	Level* myLevel = nullptr;
	Slush::EntityPrefab* myBasicTowerPrefab = nullptr;
	Slush::EntityPrefab* mySplashTowerPrefab = nullptr;
	int myPendingClusterEnemyCount = 0;
};
