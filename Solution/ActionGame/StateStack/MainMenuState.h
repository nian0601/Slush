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

	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myUIRenderCommands;
	Slush::UIRenderer myUIRenderer;
};
