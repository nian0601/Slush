#pragma once

#include "Component.h"

class PickupComponent : public Component
{
public:
	COMPONENT_HELPER("Pickup", "pickup");

	using Component::Component;

	void OnCollision(Entity& aOtherEntity) override;
};