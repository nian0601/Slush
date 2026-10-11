#pragma once

#include <UI/UIBuilder.h>

class Level;

class LevelHud
{
public:
	LevelHud(Level& aLevel);

	void Update();
	bool IsMouseOverUI() const { return myIsMouseOverUI; }
	const FW_GrowingArray<Slush::UIBuilder::RenderCommand>& GetRenderCommands() const { return myRenderCommands; }

private:
	Level& myLevel;
	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myRenderCommands;
	bool myIsMouseOverUI = false;
};
