#pragma once

#include "GameState.h"

#include "UI\UIManager.h"

namespace Slush
{
	class Font;
}

class Tilemap;
class MainMenuState : public GameState
{
public:
	MainMenuState();
	~MainMenuState();

	GameStateResult Update() override;
	void Render() override;

private:
	Tilemap* myTilemap;

	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::DynamicUIBuilder::RenderCommand> myUIRenderCommands;
	Slush::DynamicUIRenderer myUIRenderer;
	Slush::Font& myFont;
};
