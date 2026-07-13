#pragma once

#include "EntitySystem/Component.h"

#include <FW_Vector2.h>

namespace Slush
{
	class PhysicsWorld;

	struct PhysicsObject;
}

class PhysicsComponent : public Slush::Component
{
public:
	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		bool myStatic = false;
		bool mySensor = false;
		bool myMatchSprite = true;
		float myRadius = 10.f;
		Vector2f mySize;
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
