#pragma once

#include "StateStack/IGameState.h"

class Level;
class LevelData;

// Deterministic tower placement/attack test harness, started with the -towertest flag.
class TowerTestState : public Slush::IGameState
{
public:
	TowerTestState(LevelData& aLevelData);
	~TowerTestState();

	GameStateResult Update() override;
	void Render() override;

private:
	Level* myLevel = nullptr;
};
