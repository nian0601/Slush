#pragma once

#include "EntityHandle.h"
#include <UI\UILayout.h>
#include <UI\UIManager.h>

class EntityManager;
class NPCWave;
class Tilemap;

namespace Slush
{
	class PhysicsWorld;
	class UIButton;
	class UIManager;
	class Font;
	class RectSprite;
}

class Level
{
public:
	Level(Slush::Font& aFont, Slush::AssetStorage<Slush::UILayout>& someUILayouts);
	~Level();

	void Update();

	void RenderGame();
	void RenderUI();

	bool IsPlayerDead() const;
	bool IsLevelingUp() const { return myIsLevelingUp; }

private:
	Slush::PhysicsWorld& myPhysicsWorld;
	EntityManager& myEntityManager;
	EntityHandle myPlayerHandle;

	NPCWave* myNPCWave;
	bool myIsLevelingUp = false;


	Slush::UIButton* myDamageUpgradeButton;
	Slush::UIButton* myCooldownUpgradeButton;
	Slush::UIButton* myProjectileUpgradeButton;
	Slush::UIManager* myUIManager;


	FW_GrowingArray<Slush::DynamicUIBuilder::RenderCommand> myUIRenderCommands;
	Slush::RectSprite* myUISprite;

	Tilemap* myTilemap;
};
