#pragma once

#include "StateStack/IGameState.h"
#include "UI/UIBuilder.h"

namespace Slush
{
	class Font;
}

class VictoryState : public Slush::IGameState
{
public:
	VictoryState();

	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; };

private:
	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myUIRenderCommands;
	Slush::UIRenderer myUIRenderer;
};
