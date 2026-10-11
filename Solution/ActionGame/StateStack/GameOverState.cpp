#include "stdafx.h"

#include "GameOverState.h"
#include "ActionGameGlobals.h"
#include "Level\Level.h"
#include "Graphics\Window.h"
#include "Graphics\Renderer.h"

GameOverState::GameOverState(Level& aLevel)
	: myLevel(aLevel)
	, myUIRenderer(ActionGameGlobals::GetInstance().GetFont())
{

	Slush::Engine::GetInstance().GetWindow().GetRenderer().StartFade(0.15f);
}

Slush::IGameState::GameStateResult GameOverState::Update()
{
	Slush::UIBuilder uiBuilder;
	uiBuilder.Start(ActionGameGlobals::GetInstance().GetUITheme());
	uiBuilder.ScreenFade();
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start(ActionGameGlobals::GetInstance().GetUITheme());
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);

		uiBuilder.Text("Game Over!", Slush::UITextRole::Title);
		uiBuilder.VerticalSpacing(60);
		uiBuilder.Button("Try Again?", Slush::UIState::Normal, { 250, 75 });

		uiBuilder.CloseElement();
	}

	uiBuilder.Finish(myUIRenderCommands);

	if (uiBuilder.WasClicked("Try Again?"))
	{
		myLevel.Restart();
		return Slush::IGameState::POP_SUBSTATE;
	}

	return Slush::IGameState::KEEP;
}

void GameOverState::Render()
{
	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}