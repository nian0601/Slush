#include "stdafx.h"

#include "ActionGameGlobals.h"
#include "MainMenuState.h"
#include "LevelState.h"
#include "Level\Tilemap.h"
#include "StateStack/StateStack.h"
#include "Graphics\Window.h"
#include "CharacterSelectionState.h"

MainMenuState::MainMenuState()
	: myUIRenderer(ActionGameGlobals::GetInstance().GetFont())
{
	myTilemap = new Tilemap();

}

MainMenuState::~MainMenuState()
{
	FW_SAFE_DELETE(myTilemap);
}

Slush::IGameState::GameStateResult MainMenuState::Update()
{
	if (ActionGameGlobals::GetInstance().myDebugSettings.mySkipStartScreen)
	{
		myStateStack->PushMainState(new CharacterSelectionState());
		return Slush::IGameState::KEEP;
	}

	Slush::UIBuilder uiBuilder;
	uiBuilder.Start(ActionGameGlobals::GetInstance().GetUITheme());
	uiBuilder.ScreenFade();
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start(ActionGameGlobals::GetInstance().GetUITheme());
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);

		uiBuilder.Text("Action Game!", Slush::UITextRole::Title);
		uiBuilder.VerticalSpacing(60);
		uiBuilder.Button("Start Game", Slush::UIState::Normal, { 250, 75 });
		uiBuilder.VerticalSpacing(20);
		uiBuilder.Button("Quit", Slush::UIState::Normal, { 250, 75 });

		uiBuilder.CloseElement();
	}

	uiBuilder.Finish(myUIRenderCommands);

	if (uiBuilder.WasClicked("Start Game"))
	{
		myStateStack->PushMainState(new CharacterSelectionState());
	}
	else if (uiBuilder.WasClicked("Quit"))
	{
		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.Close();
	}

	return Slush::IGameState::KEEP;
}

void MainMenuState::Render()
{
	myTilemap->Render();

	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}

