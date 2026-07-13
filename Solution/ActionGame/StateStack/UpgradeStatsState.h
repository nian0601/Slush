#pragma once

#include "GameState.h"
#include "UI\UIManager.h"
#include "EntitySystem\EntityHandle.h"
#include "Components\StatsComponent.h"

namespace Slush
{
	class Font;
}

class UpgradeStatsState : public GameState
{
public:
	UpgradeStatsState(Slush::EntityHandle aPlayerHandle);

	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; };

private:
	Slush::EntityHandle myPlayerHandle;
	FW_GrowingArray<StatType> myUpgradeOptions;
	FW_GrowingArray<FW_String> myUpgradeLabels;

	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::DynamicUIBuilder::RenderCommand> myUIRenderCommands;
	Slush::DynamicUIRenderer myUIRenderer;
	Slush::Font& myFont;
};
