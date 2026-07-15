#pragma once

#include "StateStack/IGameState.h"
#include "UI\UIManager.h"

namespace Slush
{
	class Font;
}

class Level;
class GameOverState : public Slush::IGameState
{
public:
	GameOverState(Level& aLevel);

	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; };

private:
	Level& myLevel;

	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::DynamicUIBuilder::RenderCommand> myUIRenderCommands;
	Slush::DynamicUIRenderer myUIRenderer;
	Slush::Font& myFont;
};
