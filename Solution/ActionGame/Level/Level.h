#pragma once


#include <UI\UIManager.h>

class EntityManager;
class Tilemap;
class LevelData;

struct EnemyWaveData;

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
	void HandleLevlingUp();
	void HandleEnemyWaves();

	bool IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance);	

	LevelData* myLevelData = nullptr;
	EnemyWaveData* myEnemyWaveData = nullptr;
	EntityManager& myEntityManager;
	EntityHandle myPlayerHandle;
	bool myIsLevelingUp = false;

	Tilemap* myTilemap;

	Slush::Timer myWaveTimer;
	int myWaveCounter = 0;
	float myPlayerClearanceRadius = 350.f;
	float myEnemyClearanceRadius = 100.f;

	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::DynamicUIBuilder::RenderCommand> myUIRenderCommands;
	Slush::RectSprite* myUISprite;
	Slush::Text* myText;
	Slush::Font& myFont;

};
