#include "stdafx.h"

#include "MainMenuState.h"
#include "LevelSelectState.h"
#include "TopDownGameGlobals.h"
#include "Graphics/Window.h"
#include "StateStack/StateStack.h"

MainMenuState::MainMenuState()
	: myUIRenderer(TopDownGameGlobals::GetInstance().GetFont())
{
}

Slush::IGameState::GameStateResult MainMenuState::Update()
{
	Slush::UIBuilder uiBuilder;
	uiBuilder.Start(TopDownGameGlobals::GetInstance().GetUITheme());
	uiBuilder.ScreenFade();
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start(TopDownGameGlobals::GetInstance().GetUITheme());
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
		uiBuilder.Text("Top Down Game", Slush::UITextRole::Title);
		uiBuilder.VerticalSpacing(60);
		uiBuilder.Button("Start", Slush::UIState::Normal, { 250, 75 });
		uiBuilder.VerticalSpacing(20);
		uiBuilder.Button("Quit", Slush::UIState::Normal, { 250, 75 });
		uiBuilder.CloseElement();
	}
	uiBuilder.Finish(myUIRenderCommands);

	if (uiBuilder.WasClicked("Start"))
	{
		myStateStack->PushMainState(new LevelSelectState());
	}
	else if (uiBuilder.WasClicked("Quit"))
	{
		Slush::Engine::GetInstance().GetWindow().Close();
	}

	return Slush::IGameState::KEEP;
}

void MainMenuState::Render()
{
	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}
