#include "stdafx.h"

#include "LevelState.h"

#include "Level/Level.h"
#include "Level/LevelData.h"
#include "TopDownGameGlobals.h"
#include "TowerBuildMenu.h"

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
	// Freeze on a resolved result while Render keeps drawing the level; #94/#95 push their end screens here.
	if (myLevel->GetResult() == LevelResult::InProgress)
	{
		myLevel->Update();
		myTowerBuildMenu->Update();
	}
	return Slush::IGameState::KEEP;
}

void LevelState::Render()
{
	myLevel->Render();
	myTowerBuildMenu->RenderPreview();
	myUIRenderer.Render(myTowerBuildMenu->GetRenderCommands());
}
