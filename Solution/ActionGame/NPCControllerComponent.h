#pragma once

#include "Component.h"
#include "EntityHandle.h"

class NPCControllerComponent : public Component
{
public:
	using Component::Component;

	void Update();

private:
	float myMaxDistance = 800.f;
};
