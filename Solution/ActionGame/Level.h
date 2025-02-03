#pragma once

#include "EntityHandle.h"
#include <UI\UILayout.h>

class EntityManager;
class NPCWave;

namespace Slush
{
	class PhysicsWorld;
	class UIButton;
	class UIManager;
	class Font;
}

class Level
{
public:
	Level(EntityManager& aEntityManager, Slush::PhysicsWorld& aPhysicsWorld, Slush::Font& aFont, Slush::AssetStorage<Slush::UILayout>& someUILayouts);
	~Level();

	void Update();
	void Render();

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
};
