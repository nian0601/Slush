#pragma once

#include "GameState.h"
#include "UI\UIManager.h"
#include "EntitySystem\EntityHandle.h"

namespace Slush
{
	class Font;
}

class Tilemap;
class CharacterSelectionState : public GameState
{
public:
	CharacterSelectionState();
	~CharacterSelectionState();

	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; }

private:
	void BuildCharacterDisplay(Slush::DynamicUIBuilder& aUIBUilder);

	Tilemap* myTilemap;

	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::DynamicUIBuilder::RenderCommand> myUIRenderCommands;
	Slush::DynamicUIRenderer myUIRenderer;
	Slush::Font& myFont;
};
