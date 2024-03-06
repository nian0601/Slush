#pragma once

#include "Component.h"

#include <FW_Vector2.h>

namespace Slush
{
	struct PhysicsObject;
}

class PhysicsComponent : public Component
{
public:
	using Component::Component;

	void Update();

	Slush::PhysicsObject* myObject;
};
