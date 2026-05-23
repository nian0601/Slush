#pragma once

#include "GameState.h"
#include "UI\UIManager.h"

namespace Slush
{
	class Font;
}

class GameOverState : public GameState
{
public:
	GameOverState();
	~GameOverState();

	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; };

private:
	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::DynamicUIBuilder::RenderCommand> myUIRenderCommands;
	Slush::DynamicUIRenderer myUIRenderer;
	Slush::Font& myFont;
};
