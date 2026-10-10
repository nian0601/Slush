#include "stdafx.h"

#include "GameLayout.h"

#include "Level/LevelData.h"
#include "StateStack/LevelState.h"
#include "StateStack/MainMenuState.h"
#include "StateStack/LevelSelectState.h"
#include "StateStack/TowerTestState.h"

#include "Core/Assets/AssetStorage.h"
#include "Core/CommandLineArgs.h"
#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/LogDockable.h"
#include "Core/Engine.h"
#include "Graphics/Renderer.h"
#include "Graphics/Window.h"
#include "StateStack/StateStack.h"

namespace
{
	const char* const ourTowerTestLevelDataAssetName = "level_test";
}

GameLayout::GameLayout()
	: Slush::IAppLayout("Game")
{
	myStateStack = new Slush::StateStack();

	Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
	Slush::CommandLineArgs& commandLineArgs = Slush::CommandLineArgs::GetInstance();

	// Checked before normal level selection so the test harness stays reachable once #88's main menu lands.
	if (commandLineArgs.HasFlag("-towertest"))
	{
		LevelData* testLevelData = assetRegistry.GetAsset<LevelData>(ourTowerTestLevelDataAssetName);
		FW_ASSERT(testLevelData, "-towertest has no valid LevelData - expected a 'level_test' LevelData asset");
		myStateStack->PushMainState(new TowerTestState(*testLevelData));
	}
	else
	{
		myStateStack->PushMainState(new MainMenuState());
		if (commandLineArgs.HasFlag("-level"))
		{
			if (const char* levelAssetName = commandLineArgs.GetString("-level"))
			{
				LevelData* levelData = assetRegistry.GetAsset<LevelData>(levelAssetName);
				FW_ASSERT(levelData, "-level has no valid LevelData - requested '%s'", levelAssetName);
				myStateStack->PushMainState(new LevelState(*levelData));
			}
			else
			{
				SLUSH_WARNING("Missing -level value; showing the main menu");
			}
		}
		else if (commandLineArgs.HasFlag("-levelselect"))
		{
			myStateStack->PushMainState(new LevelSelectState());
		}
	}

	AddDockable(new Slush::GameViewDockable());
	AddDockable(new Slush::LogDockable());
}

GameLayout::~GameLayout()
{
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
