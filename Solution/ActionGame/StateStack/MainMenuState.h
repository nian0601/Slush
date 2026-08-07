#pragma once

#include "StateStack/IGameState.h"

#include "UI\UIBuilder.h"

namespace Slush
{
	class Font;
}

class Tilemap;
class MainMenuState : public Slush::IGameState
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
	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myUIRenderCommands;
	Slush::UIRenderer myUIRenderer;
	Slush::Font& myFont;
};
