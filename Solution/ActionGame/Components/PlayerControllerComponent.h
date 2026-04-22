#pragma once

#include "Component.h"

#include <FW_Vector2.h>

namespace Slush
{
	class Animation;
}

class PlayerControllerComponent : public Component
{
public:
	COMPONENT_HELPER("Player Controller", "playercontroller");

	using Component::Component;

	void OnEnterWorld() override;
	void PrePhysicsUpdate() override;

private:
	Vector2f myDirection;
	float mySpeed = 400.f;

	const Slush::Animation* myDashAnimation;
	const Slush::Animation* myBlinkAnimation;
	const Slush::Animation* mySpriteSheetAnimation;
};
