#pragma once

#include <UI/UIBuilder.h>
#include <Core/Time.h>

class Level;
namespace Slush { class EntityPrefab; }

class TowerBuildMenu
{
public:
	TowerBuildMenu(Level& aLevel);

	void Update();
	void RenderPreview() const;
	const FW_GrowingArray<Slush::UIBuilder::RenderCommand>& GetRenderCommands() const { return myRenderCommands; }

private:
	static const int ourBarHeight = 100;

	Level& myLevel;
	const Slush::EntityPrefab* mySelectedTowerPrefab = nullptr;
	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myRenderCommands;
	FW_String myFeedbackText;
	Slush::Timer myFeedbackTimer;
};
