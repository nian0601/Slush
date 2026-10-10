#pragma once

#include "StateStack/IGameState.h"
#include <UI/UIBuilder.h>

class Level;
class LevelData;
class TowerBuildMenu;

class LevelState : public Slush::IGameState
{
public:
	LevelState(LevelData& aLevelData);
	~LevelState();

	GameStateResult Update() override;
	void Render() override;

	void RestartLevel();

private:
	LevelData& myLevelData;
	Level* myLevel = nullptr;
	TowerBuildMenu* myTowerBuildMenu = nullptr;
	Slush::UIRenderer myUIRenderer;
};
