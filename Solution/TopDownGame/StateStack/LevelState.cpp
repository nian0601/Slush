#include "stdafx.h"

#include "LevelState.h"

#include "Level/Level.h"
#include "Level/LevelData.h"

LevelState::LevelState(LevelData& aLevelData)
{
	myLevel = new Level(aLevelData);
}

LevelState::~LevelState()
{
	FW_SAFE_DELETE(myLevel);
}

Slush::IGameState::GameStateResult LevelState::Update()
{
	myLevel->Update();
	return Slush::IGameState::KEEP;
}

void LevelState::Render()
{
	myLevel->Render();
}
