#pragma once

#include "EntitySystem/Component.h"

#include <FW_Vector2.h>

namespace Slush
{
	class PhysicsWorld;

	struct PhysicsObject;
}

namespace CollisionUtils
{
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

	const char* ToString(CollisionFlag aFlag);
	const char* const* GetNames();
	const char* const* GetSerializationNames();
}

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

		int myCollisionFlag = CollisionUtils::COL_ENVIRONMENT;
		FW_StaticArray<bool, CollisionUtils::COLLISIONFLAG_COUNT> myCollidesWithFlags;
	};

public:
	COMPONENT_HELPER("Physics", "physics");

	PhysicsComponent(Slush::Entity& aEntity, const Slush::EntityPrefab& anEntityPrefab);
	~PhysicsComponent();

	void Update() override;

	CollisionUtils::CollisionFlag GetCollisionFlag() const;

	Slush::PhysicsObject* myObject;

private:
	Slush::PhysicsWorld& myPhysicsWorld;
};
