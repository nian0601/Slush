#include "stdafx.h"

#include "GameOverState.h"
#include "ActionGameGlobals.h"

GameOverState::GameOverState()
	: myFont(ActionGameGlobals::GetInstance().GetFont())
	, myUIRenderer(ActionGameGlobals::GetInstance().GetFont())
{
	myUIBackgroundStyle.SetPadding(16, 16);
	myUIBackgroundStyle.SetChildGap(16);
	myUIBackgroundStyle.SetColor(0xAA121212);
	myUIBackgroundStyle.SetXSizing(Slush::UIElementStyle::FIT);
	myUIBackgroundStyle.SetAlingment(Slush::UIElementStyle::CENTER);

	myUIButtonStyle.SetXSizing(Slush::UIElementStyle::FIXED, 250);
	myUIButtonStyle.SetYSizing(Slush::UIElementStyle::FIXED, 75);
	myUIButtonStyle.SetAlingment(Slush::UIElementStyle::CENTER);
	myUIButtonStyle.SetColor(0xFFAAFFAF);
	myUIButtonStyle.SetOutlineThickness(-1.f);
	myUIButtonStyle.EnableButtonInteraction(0xFFDDDDDD);
}

GameOverState::~GameOverState()
{

}

GameState::GameStateResult GameOverState::Update()
{
	Slush::DynamicUIBuilder uiBuilder;
	uiBuilder.Start();
	uiBuilder.ScreenFade(myUIBackgroundStyle.myColor);
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start();
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);

		uiBuilder.Text("Game Over!", myFont, 50);
		uiBuilder.VerticalSpacing(60);
		uiBuilder.Button("Try Again?", myFont, 25, myUIButtonStyle, 0xFFFF3333, 0xFF000000);

		uiBuilder.CloseElement();
	}

	uiBuilder.Finish(myUIRenderCommands);

	if (uiBuilder.WasClicked("Try Again?"))
		return GameState::POP_SUBSTATE;

	return GameState::KEEP;
}

void GameOverState::Render()
{
	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}