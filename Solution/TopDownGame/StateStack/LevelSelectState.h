#pragma once

#include "StateStack/IGameState.h"
#include "UI/UIBuilder.h"

namespace Slush
{
	class Font;
}

class LevelSelectState : public Slush::IGameState
{
public:
	LevelSelectState();

	GameStateResult Update() override;
	void Render() override;

private:
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myUIRenderCommands;
	Slush::UIRenderer myUIRenderer;
	Slush::Font& myFont;
};
