#pragma once

#include "Component.h"

#include <FW_Vector2.h>

namespace Slush
{
	class PhysicsWorld;

	struct PhysicsObject;
}

class EntityPrefab;
class PhysicsComponent : public Component
{
public:
	struct Data : public Component::BaseData
	{
		Data();

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		bool myStatic = false;
		bool mySensor = false;
		bool myMatchSprite = true;
		float myRadius = 10.f;
		Vector2f mySize;
	};

public:
	PhysicsComponent(Entity& aEntity, const EntityPrefab& anEntityPrefab);
	~PhysicsComponent();

	void Update() override;

	Slush::PhysicsObject* myObject;

private:
	Slush::PhysicsWorld& myPhysicsWorld;
};
