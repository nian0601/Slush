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
// F8 runs independent snap/contact/resource checks without changing the F1-F7 scenario.
class TowerTestState : public Slush::IGameState
{
public:
	const char* GetTriggerName() const override { return "towertest"; }
	TowerTestState(LevelData& aLevelData);
	~TowerTestState();

	GameStateResult Update() override;
	void Render() override;

private:
	void UpdateScenarioKeys();
	void RunPlacementScenario(const char* aName, const Slush::EntityPrefab& aTowerPrefab, const Vector2f& aPosition, PlaceTowerResult anExpectedResult);
	void RunSnapPlacementScenarios();

	Level* myLevel = nullptr;
	Slush::EntityPrefab* myBasicTowerPrefab = nullptr;
	Slush::EntityPrefab* mySplashTowerPrefab = nullptr;
	int myPendingClusterEnemyCount = 0;
};
