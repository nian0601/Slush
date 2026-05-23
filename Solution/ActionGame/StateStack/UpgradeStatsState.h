#pragma once

#include "GameState.h"
#include "UI\UIManager.h"
#include "EntitySystem\EntityHandle.h"

namespace Slush
{
	class Font;
}

class UpgradeStatsState : public GameState
{
public:
	UpgradeStatsState(EntityHandle aPlayerHandle);

	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; };

private:
	EntityHandle myPlayerHandle;

	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::DynamicUIBuilder::RenderCommand> myUIRenderCommands;
	Slush::DynamicUIRenderer myUIRenderer;
	Slush::Font& myFont;
};
