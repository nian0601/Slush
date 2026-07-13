#pragma once

#include "EntitySystem/Component.h"

#include <FW_Vector2.h>

namespace Slush
{
	class PhysicsWorld;

	struct PhysicsObject;
}

enum CollisionFlag
{
	COL_ENVIRONMENT,
	COL_PLAYER,
	COL_NPC,
	COL_PLAYER_PROJECTILE,
	COL_NPC_PROJECTILE,
	COL_PICKUP,
	COLLISIONFLAG_COUNT,
};

class PhysicsComponent : public Slush::Component
{
public:
	struct Data : public Slush::Component::BaseData
	{
		Data() { myCollidesWithFlags.Fill(false); }

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		bool myStatic = false;
		bool mySensor = false;
		bool myMatchSprite = true;
		float myRadius = 10.f;
		Vector2f mySize;

		int myCollisionFlag = CollisionFlag::COL_ENVIRONMENT;
		FW_StaticArray<bool, CollisionFlag::COLLISIONFLAG_COUNT> myCollidesWithFlags;
	};

public:
	COMPONENT_HELPER("Physics", "physics");

	PhysicsComponent(Slush::Entity& aEntity, const Slush::EntityPrefab& anEntityPrefab);
	~PhysicsComponent();

	void Update() override;

	Slush::PhysicsObject* myObject;

private:
	Slush::PhysicsWorld& myPhysicsWorld;
};
