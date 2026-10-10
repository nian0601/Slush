#include "stdafx.h"

#include "LevelState.h"

#include "Level/Level.h"
#include "Level/LevelData.h"
#include "TopDownGameGlobals.h"
#include "TowerBuildMenu.h"
#include "VictoryState.h"
#include "GameOverState.h"

#include "StateStack/StateStack.h"

LevelState::LevelState(LevelData& aLevelData)
	: myUIRenderer(TopDownGameGlobals::GetInstance().GetFont())
{
	myLevel = new Level(aLevelData, true);
	myTowerBuildMenu = new TowerBuildMenu(*myLevel);
}

LevelState::~LevelState()
{
	FW_SAFE_DELETE(myTowerBuildMenu);
	FW_SAFE_DELETE(myLevel);
}

Slush::IGameState::GameStateResult LevelState::Update()
{
	// Freeze on a resolved result while Render keeps drawing the level behind the end screen.
	if (myLevel->GetResult() == LevelResult::InProgress)
	{
		myLevel->Update();
		myTowerBuildMenu->Update();
	}
	if (myLevel->GetResult() == LevelResult::Won)
	{
		myStateStack->PushSubState(new VictoryState());
	}
	else if (myLevel->GetResult() == LevelResult::Lost)
	{
		myStateStack->PushSubState(new GameOverState());
	}
	return Slush::IGameState::KEEP;
}

void LevelState::Render()
{
	myLevel->Render();
	if (myLevel->GetResult() == LevelResult::InProgress)
	{
		myTowerBuildMenu->RenderPreview();
	}
	myUIRenderer.Render(myTowerBuildMenu->GetRenderCommands());
}
