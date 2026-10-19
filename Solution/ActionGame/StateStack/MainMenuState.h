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
	const char* GetTriggerName() const override { return "mainmenu"; }
	MainMenuState();
	~MainMenuState();

	GameStateResult Update() override;
	void Render() override;

private:
	Tilemap* myTilemap;

	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myUIRenderCommands;
	Slush::UIRenderer myUIRenderer;
};
