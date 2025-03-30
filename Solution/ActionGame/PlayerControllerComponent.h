#pragma once

#include "Component.h"

#include <FW_Vector2.h>

class PlayerControllerComponent : public Component
{
public:
	COMPONENT_HELPER("Player Controller", "playercontroller");

	using Component::Component;

	void PrePhysicsUpdate() override;

private:
	Vector2f myDirection;
	float mySpeed = 400.f;
};
