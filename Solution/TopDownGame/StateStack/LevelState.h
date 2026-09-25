#pragma once

#include "StateStack/IGameState.h"

class Level;
class LevelData;

class LevelState : public Slush::IGameState
{
public:
	LevelState(LevelData& aLevelData);
	~LevelState();

	GameStateResult Update() override;
	void Render() override;

private:
	Level* myLevel = nullptr;
};
