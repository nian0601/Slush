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
		window.AddDockable(new Slush::GameViewDockable());
		window.AddDockable(new Slush::LogDockable());
		window.AddDockable(new ActionGameGlobals::DebugSettingsDockable());
	}
}

GameLayout::~GameLayout()
{
	myStateStack->Clear();
	FW_SAFE_DELETE(myStateStack);
}

void GameLayout::Update()
{
	myStateStack->Update();
}

void GameLayout::Render()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.GetWindow().StartOffscreenBuffer();

	myStateStack->Render();

	engine.GetWindow().EndOffscreenBuffer();
}
