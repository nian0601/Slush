#pragma once

#include "StateStack/IGameState.h"
#include "UI/UIBuilder.h"

namespace Slush
{
	class Font;
}

class LevelData;

class LevelSelectState : public Slush::IGameState
{
public:
	const char* GetTriggerName() const override { return "levelselect"; }
	LevelSelectState();

	void StartState() override;
	GameStateResult Update() override;
	void Render() override;

private:
	FW_GrowingArray<LevelData*> myLevels;
	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myUIRenderCommands;
	Slush::UIRenderer myUIRenderer;
};
