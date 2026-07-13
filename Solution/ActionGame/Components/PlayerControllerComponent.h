#pragma once

#include "EntitySystem/Component.h"

#include <FW_Vector2.h>

namespace Slush
{
	class Animation;
}

class PlayerControllerComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Player Controller", "playercontroller");

	using Slush::Component::Component;

	void OnEnterWorld() override;
	void PrePhysicsUpdate() override;

private:
	Vector2f myDirection;
	float mySpeed = 400.f;

	const Slush::Animation* myDashAnimation;
	const Slush::Animation* myBlinkAnimation;
	const Slush::Animation* mySpriteSheetAnimation;
};
