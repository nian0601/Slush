#pragma once

#include "Component.h"

class PickupComponent : public Component
{
public:
	using Component::Component;

	void OnCollision(Entity& aOtherEntity) override;
};