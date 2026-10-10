#include "stdafx.h"

#include "GameOverState.h"
#include "LevelState.h"
#include "TopDownGameGlobals.h"

#include "Core/Engine.h"
#include "Core/Input.h"

GameOverState::GameOverState(LevelState& aLevelState)
	: myLevelState(aLevelState)
	, myFont(TopDownGameGlobals::GetInstance().GetFont())
	, myUIRenderer(myFont)
{
	myUIButtonStyle.SetXSizing(Slush::UIElementStyle::FIXED, 250);
	myUIButtonStyle.SetYSizing(Slush::UIElementStyle::FIXED, 75);
	myUIButtonStyle.SetAlingment(Slush::UIElementStyle::CENTER);
	myUIButtonStyle.SetColor(0xFF334453);
	myUIButtonStyle.EnableButtonInteraction(0xFF60758A);
}

void GameOverState::StartState()
{
	SLUSH_INFO("[GameOver] Shown");
}

Slush::IGameState::GameStateResult GameOverState::Update()
{
	Slush::UIBuilder uiBuilder;
	uiBuilder.Start();
	uiBuilder.ScreenFade(0xAA121212);
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start();
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);

		uiBuilder.Text("Game Over", myFont, 50);
		uiBuilder.VerticalSpacing(60);
		uiBuilder.Button("Restart Level", myFont, 25, myUIButtonStyle, 0xFF334453, 0xFFFFFFFF);
		uiBuilder.VerticalSpacing(20);
		uiBuilder.Button("Main Menu", myFont, 25, myUIButtonStyle, 0xFF334453, 0xFFFFFFFF);

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
