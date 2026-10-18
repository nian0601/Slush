#pragma once

#include "StateStack/IGameState.h"
#include "UI\UIBuilder.h"

namespace Slush
{
	class Font;
}

class Level;
class GameOverState : public Slush::IGameState
{
public:
	const char* GetTriggerName() const override { return "gameover"; }
	GameOverState(Level& aLevel);

	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; };

private:
	Level& myLevel;

	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myUIRenderCommands;
	Slush::UIRenderer myUIRenderer;
};
