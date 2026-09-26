#include "stdafx.h"

#include "TowerTestState.h"

#include "Level/Level.h"
#include "Level/LevelData.h"

TowerTestState::TowerTestState(LevelData& aLevelData)
{
	myLevel = new Level(aLevelData);
}

TowerTestState::~TowerTestState()
{
	FW_SAFE_DELETE(myLevel);
}

Slush::IGameState::GameStateResult TowerTestState::Update()
{
	myLevel->Update();
	return Slush::IGameState::KEEP;
}

void TowerTestState::Render()
{
	myLevel->Render();
}
