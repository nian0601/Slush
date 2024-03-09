#pragma once

#include "Component.h"

class RemoveOnCollisionComponent : public Component
{
public:
	using Component::Component;

	void OnCollision(Entity& aOtherEntity);
};