#include "stdafx.h"

#include "Graphics\Window.h"
#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/TextureViewerDockable.h"
#include "Core/Dockables/LogDockable.h"
#include "Core/Dockables/ContentBrowserDockable.h"

#include "ActionGameGlobals.h"
#include "GameLayout.h"
#include "StateStack\StateStack.h"
#include "StateStack\MainMenuState.h"

GameLayout::GameLayout()
	: Slush::IAppLayout("Game")
{
	myStateStack = new StateStack();
	myStateStack->PushMainState(new MainMenuState());
	
	bool disableEditorStuff = false;
	Slush::Window& window = Slush::Engine::GetInstance().GetWindow();

	if (disableEditorStuff)
	{
		window.ToggleEditorUI();
	}
	else
	{
		AddDockable(new Slush::GameViewDockable());
		AddDockable(new Slush::LogDockable());
		AddDockable(new ActionGameGlobals::DebugSettingsDockable());
	}
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
	engine.GetWindow().StartOffscreenBuffer();

	myStateStack->Render();

	engine.GetWindow().EndOffscreenBuffer();
}
