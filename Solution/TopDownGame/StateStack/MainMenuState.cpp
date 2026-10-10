#include "stdafx.h"

#include "MainMenuState.h"
#include "LevelSelectState.h"
#include "TopDownGameGlobals.h"
#include "Graphics/Window.h"
#include "StateStack/StateStack.h"

MainMenuState::MainMenuState()
	: myUIRenderer(TopDownGameGlobals::GetInstance().GetFont())
	, myFont(TopDownGameGlobals::GetInstance().GetFont())
{
	myUIButtonStyle.SetXSizing(Slush::UIElementStyle::FIXED, 250);
	myUIButtonStyle.SetYSizing(Slush::UIElementStyle::FIXED, 75);
	myUIButtonStyle.SetAlingment(Slush::UIElementStyle::CENTER);
	myUIButtonStyle.SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
	myUIButtonStyle.SetChildGap(8);
	myUIButtonStyle.SetPadding(16, 16);
	myUIButtonStyle.SetColor(0xFF333333);
	myUIButtonStyle.SetOutlineColor(0xFF000000);
	myUIButtonStyle.SetOutlineThickness(-1.f);
	myUIButtonStyle.EnableButtonInteraction(0xFF888888);
}

Slush::IGameState::GameStateResult MainMenuState::Update()
{
	Slush::UIBuilder uiBuilder;
	uiBuilder.Start();
	uiBuilder.ScreenFade(0xAA121212);
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start();
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
		uiBuilder.Text("Top Down Game", myFont, 50);
		uiBuilder.VerticalSpacing(60);
		uiBuilder.Button("Start", myFont, 25, myUIButtonStyle, 0xFF333333, 0xFFFFFFFF);
		uiBuilder.VerticalSpacing(20);
		uiBuilder.Button("Quit", myFont, 25, myUIButtonStyle, 0xFF333333, 0xFFFFFFFF);
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
