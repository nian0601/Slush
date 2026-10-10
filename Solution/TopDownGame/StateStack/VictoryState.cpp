#include "stdafx.h"

#include "VictoryState.h"
#include "TopDownGameGlobals.h"

VictoryState::VictoryState()
	: myFont(TopDownGameGlobals::GetInstance().GetFont())
	, myUIRenderer(myFont)
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

	SLUSH_INFO("[Victory] Shown");
}

Slush::IGameState::GameStateResult VictoryState::Update()
{
	Slush::UIBuilder uiBuilder;
	uiBuilder.Start();
	uiBuilder.ScreenFade(myUIBackgroundStyle.myColor);
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start();
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);

		uiBuilder.Text("Victory!", myFont, 50);
		uiBuilder.VerticalSpacing(60);
		uiBuilder.Button("Main Menu", myFont, 25, myUIButtonStyle, 0xFFFF3333, 0xFF000000);

		uiBuilder.CloseElement();
	}

	uiBuilder.Finish(myUIRenderCommands);

	if (uiBuilder.WasClicked("Main Menu"))
	{
		SLUSH_INFO("[Victory] Main Menu clicked");
		return Slush::IGameState::POP_MAINSTATE;
	}

	return Slush::IGameState::KEEP;
}

void VictoryState::Render()
{
	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}