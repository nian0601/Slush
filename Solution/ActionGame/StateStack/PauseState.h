#pragma once

#include "StateStack/IGameState.h"
#include "UI\UIBuilder.h"
#include "EntitySystem\EntityHandle.h"

namespace Slush
{
	class Font;
}

class CharacterInfo;
class PauseState : public Slush::IGameState
{
public:
	PauseState(Slush::EntityHandle aPlayerHandle, const CharacterInfo& aCharacterInfo);

	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; };

private:
	void BuildStatsDisplay(Slush::UIBuilder& aUIBUilder);
	void BuildWeaponsDisplay(Slush::UIBuilder& aUIBUilder);
	void BuildCharacterDisplay(Slush::UIBuilder& aUIBuilder);
	void BuildNavigationButtons(Slush::UIBuilder& aUIBUilder);

	Slush::EntityHandle myPlayerHandle;
	const CharacterInfo& myCharacterInfo;

	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myUIRenderCommands;
	Slush::UIRenderer myUIRenderer;
	Slush::Font& myFont;
};
