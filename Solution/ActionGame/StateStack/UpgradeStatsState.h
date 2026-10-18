#pragma once

#include "StateStack/IGameState.h"
#include "UI\UIBuilder.h"
#include "EntitySystem\EntityHandle.h"
#include "Components\StatsComponent.h"

namespace Slush
{
	class Font;
}

class UpgradeStatsState : public Slush::IGameState
{
public:
	const char* GetTriggerName() const override { return "upgradestats"; }
	UpgradeStatsState(Slush::EntityHandle aPlayerHandle);

	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; };

private:
	Slush::EntityHandle myPlayerHandle;
	FW_GrowingArray<StatType> myUpgradeOptions;
	FW_GrowingArray<FW_String> myUpgradeLabels;

	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myUIRenderCommands;
	Slush::UIRenderer myUIRenderer;
};
