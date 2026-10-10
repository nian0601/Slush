#pragma once

#include "StateStack/IGameState.h"
#include "UI/UIBuilder.h"

namespace Slush
{
	class Font;
}

class LevelState;

class GameOverState : public Slush::IGameState
{
public:
	GameOverState(LevelState& aLevelState);

	void StartState() override;
	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; }

private:
	LevelState& myLevelState;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myUIRenderCommands;
	Slush::Font& myFont;
	Slush::UIRenderer myUIRenderer;
};
