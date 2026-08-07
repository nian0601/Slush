#pragma once

#include "StateStack/IGameState.h"
#include "UI\UIBuilder.h"
#include "EntitySystem\EntityHandle.h"

namespace Slush
{
	class Font;
}

class Tilemap;
class CharacterSelectionState : public Slush::IGameState
{
public:
	CharacterSelectionState();
	~CharacterSelectionState();

	GameStateResult Update() override;
	void Render() override;
	bool AllowPassThroughRender() override { return true; }

private:
	void BuildCharacterDisplay(Slush::UIBuilder& aUIBUilder);

	Tilemap* myTilemap;

	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::UIBuilder::RenderCommand> myUIRenderCommands;
	Slush::UIRenderer myUIRenderer;
	Slush::Font& myFont;
};
