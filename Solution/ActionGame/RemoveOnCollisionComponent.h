#pragma once

#include "Component.h"

class RemoveOnCollisionComponent : public Component
{
public:
	COMPONENT_HELPER("Remove On Collision", "removeoncollision");

	using Component::Component;

	void OnCollision(Entity& aOtherEntity) override;
};