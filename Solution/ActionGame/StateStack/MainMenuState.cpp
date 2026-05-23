#include "stdafx.h"

#include "ActionGameGlobals.h"
#include "MainMenuState.h"
#include "LevelState.h"
#include "Level\Tilemap.h"
#include "StateStack.h"

MainMenuState::MainMenuState()
	: myFont(ActionGameGlobals::GetInstance().GetFont())
	, myUIRenderer(ActionGameGlobals::GetInstance().GetFont())
{
	myTilemap = new Tilemap();

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

MainMenuState::~MainMenuState()
{
	FW_SAFE_DELETE(myTilemap);
}

GameState::GameStateResult MainMenuState::Update()
{
	if (ActionGameGlobals::GetInstance().myDebugSettings.mySkipStartScreen)
	{
		myStateStack->PushMainState(new LevelState());
		return GameState::KEEP;
	}

	Slush::DynamicUIBuilder uiBuilder;
	uiBuilder.Start();
	uiBuilder.ScreenFade(0xAA121212);
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start();
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);

		uiBuilder.Text("Action Game!", myFont, 50);
		uiBuilder.VerticalSpacing(60);
		uiBuilder.Button("Start Game", myFont, 25, myUIButtonStyle, 0xFFFF3333, 0xFF000000);

		uiBuilder.CloseElement();
	}

	uiBuilder.Finish(myUIRenderCommands);

	if (uiBuilder.WasClicked("Start Game"))
		myStateStack->PushMainState(new LevelState());

	return GameState::KEEP;
}

void MainMenuState::Render()
{
	myTilemap->Render();

	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}

