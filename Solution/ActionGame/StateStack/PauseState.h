#pragma once

#include "GameState.h"
#include "UI\UIManager.h"
#include "EntitySystem\EntityHandle.h"

namespace Slush
{
	class Font;
}

class PauseState : public GameState
{
public:
	PauseState(EntityHandle aPlayerHandle);

	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; };

private:
	void BuildStatsDisplay(Slush::DynamicUIBuilder& aUIBUilder);
	void BuildWeaponsDisplay(Slush::DynamicUIBuilder& aUIBUilder);
	void BuildNavigationButtons(Slush::DynamicUIBuilder& aUIBUilder);

	EntityHandle myPlayerHandle;

	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::DynamicUIBuilder::RenderCommand> myUIRenderCommands;
	Slush::DynamicUIRenderer myUIRenderer;
	Slush::Font& myFont;
};
