#pragma once

#include "StateStack/IGameState.h"
#include "UI\UIBuilder.h"
#include "EntitySystem\EntityHandle.h"

namespace Slush
{
	class Font;
}

class UpgradeWeaponState : public Slush::IGameState
{
public:
	UpgradeWeaponState(Slush::EntityHandle aPlayerHandle);

	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; };

private:
	Slush::EntityHandle myPlayerHandle;

	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myUIRenderCommands;
	Slush::UIRenderer myUIRenderer;
	Slush::Font& myFont;
};
