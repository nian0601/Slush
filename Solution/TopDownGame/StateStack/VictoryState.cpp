#include "stdafx.h"

#include "VictoryState.h"
#include "TopDownGameGlobals.h"

VictoryState::VictoryState()
	: myUIRenderer(TopDownGameGlobals::GetInstance().GetFont())
{

	SLUSH_INFO("[Victory] Shown");
}

Slush::IGameState::GameStateResult VictoryState::Update()
{
	Slush::UIBuilder uiBuilder;
	uiBuilder.Start(TopDownGameGlobals::GetInstance().GetUITheme());
	uiBuilder.ScreenFade();
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start(TopDownGameGlobals::GetInstance().GetUITheme());
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);

		uiBuilder.Text("Victory!", Slush::UITextRole::Title);
		uiBuilder.VerticalSpacing(60);
		uiBuilder.Button("Main Menu", Slush::UIState::Normal, { 250, 75 });

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