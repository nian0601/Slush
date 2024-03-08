#pragma once

#include "Component.h"
#include "EntityHandle.h"

class NPCControllerComponent : public Component
{
public:
	using Component::Component;

	void Update();

	void SetTarget(EntityHandle& anEntity) { myTargetHandle = anEntity; }

private:
	EntityHandle myTargetHandle;
	float myMaxDistance = 800.f;
};
