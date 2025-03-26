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
	PhysicsComponent(Entity& aEntity, const EntityPrefab& anEntityPrefab);
	~PhysicsComponent();

	void Update() override;

	Slush::PhysicsObject* myObject;

private:
	Slush::PhysicsWorld& myPhysicsWorld;
};
