#pragma once

#include "Component.h"

class NPCControllerComponent : public Component
{
public:
	using Component::Component;

	void Update();

	void SetTarget(Entity& anEntity) { myTarget = &anEntity; }

private:
	Entity* myTarget = nullptr;
	float myMaxDistance = 800.f;
};
