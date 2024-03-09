#pragma once

#include "Component.h"

#include <FW_Vector2.h>

namespace Slush
{
	class PhysicsWorld;

	struct PhysicsObject;
}

class PhysicsComponent : public Component
{
public:
	PhysicsComponent(Entity& aEntity, Slush::PhysicsWorld& aPhysicsWorld);
	~PhysicsComponent();

	void Update();

	Slush::PhysicsObject* myObject;

private:
	Slush::PhysicsWorld& myPhysicsWorld;
};
