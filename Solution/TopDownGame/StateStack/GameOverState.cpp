#include "stdafx.h"

#include "GameOverState.h"
#include "LevelState.h"
#include "TopDownGameGlobals.h"

#include "Core/Engine.h"
#include "Core/Input.h"

GameOverState::GameOverState(LevelState& aLevelState)
	: myLevelState(aLevelState)
	, myUIRenderer(TopDownGameGlobals::GetInstance().GetFont())
{
}

void GameOverState::StartState()
{
	SLUSH_INFO("[GameOver] Shown");
}

Slush::IGameState::GameStateResult GameOverState::Update()
{
	Slush::UIBuilder uiBuilder;
	uiBuilder.Start(TopDownGameGlobals::GetInstance().GetUITheme());
	uiBuilder.ScreenFade();
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start(TopDownGameGlobals::GetInstance().GetUITheme());
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);

		uiBuilder.Text("Game Over", Slush::UITextRole::Title);
		uiBuilder.VerticalSpacing(60);
		uiBuilder.Button("Restart Level", Slush::UIState::Normal, { 250, 75 });
		uiBuilder.VerticalSpacing(20);
		uiBuilder.Button("Main Menu", Slush::UIState::Normal, { 250, 75 });

		uiBuilder.CloseElement();
	}
	uiBuilder.Finish(myUIRenderCommands);

	const Slush::Input& input = Slush::Engine::GetInstance().GetInput();
	if (uiBuilder.WasClicked("Restart Level") || input.WasKeyReleased(Slush::Input::_1))
	{
		SLUSH_INFO("[GameOver] Restart Level");
		myLevelState.RestartLevel();
		return Slush::IGameState::POP_SUBSTATE;
	}
	if (uiBuilder.WasClicked("Main Menu") || input.WasKeyReleased(Slush::Input::_2))
	{
		SLUSH_INFO("[GameOver] Main Menu");
		return Slush::IGameState::POP_MAINSTATE;
	}

	return Slush::IGameState::KEEP;
}

void GameOverState::Render()
{
	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}
