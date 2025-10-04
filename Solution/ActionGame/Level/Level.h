#pragma once


#include <UI\UIManager.h>

class EntityManager;
class NPCWave;
class Tilemap;

namespace Slush
{
	class Font;
	class RectSprite;
	class Text;
}

class Level
{
public:
	Level();
	~Level();

	void Update();

	void RenderGame();
	void RenderUI();

	bool IsPlayerDead() const;
	bool IsLevelingUp() const { return myIsLevelingUp; }

private:
	EntityManager& myEntityManager;
	EntityHandle myPlayerHandle;

	NPCWave* myNPCWave;
	bool myIsLevelingUp = false;

	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::DynamicUIBuilder::RenderCommand> myUIRenderCommands;
	Slush::RectSprite* myUISprite;
	Slush::Text* myText;
	Slush::Font& myFont;
	Tilemap* myTilemap;
};
