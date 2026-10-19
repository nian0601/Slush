#pragma once

#include "StateStack/IGameState.h"
#include <UI/UIBuilder.h>

class Level;
class LevelHud;
class LevelData;
class TowerBuildMenu;

class LevelState : public Slush::IGameState
{
public:
	const char* GetTriggerName() const override { return "level"; }
	LevelState(LevelData& aLevelData);
	~LevelState();

	GameStateResult Update() override;
	void Render() override;

	void RestartLevel();

private:
	LevelData& myLevelData;
	Level* myLevel = nullptr;
	LevelHud* myLevelHud = nullptr;
	TowerBuildMenu* myTowerBuildMenu = nullptr;
	Slush::UIRenderer myUIRenderer;
};
