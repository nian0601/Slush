#include "stdafx.h"

#include "GameLayout.h"

#include "Level/LevelData.h"
#include "StateStack/LevelState.h"

#include "Core/Assets/AssetStorage.h"
#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/LogDockable.h"
#include "Core/Engine.h"
#include "Graphics/Renderer.h"
#include "Graphics/Window.h"
#include "StateStack/StateStack.h"

namespace
{
	const char* const ourInitialLevelDataAssetName = "level_main";
}

GameLayout::GameLayout()
	: Slush::IAppLayout("Game")
{
	myStateStack = new Slush::StateStack();

	LevelData* levelData = Slush::AssetRegistry::GetInstance().GetAsset<LevelData>(ourInitialLevelDataAssetName);
	FW_ASSERT(levelData, "Game has no valid LevelData - expected a 'level_main' LevelData asset");
	// #88 and #89 replace this temporary hardcoded initial level selection.
	myStateStack->PushMainState(new LevelState(*levelData));

	AddDockable(new Slush::GameViewDockable());
	AddDockable(new Slush::LogDockable());
}

GameLayout::~GameLayout()
{
	myStateStack->Clear();
	FW_SAFE_DELETE(myStateStack);
}

void GameLayout::OnUpdate()
{
	myStateStack->Update();
}

void GameLayout::OnRender()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.GetWindow().GetRenderer().StartOffscreenBuffer();

	myStateStack->Render();

	engine.GetWindow().GetRenderer().EndOffscreenBuffer();
}
