#pragma once

#include "Component.h"
#include "EntityHandle.h"

class NPCControllerComponent : public Component
{
public:
	COMPONENT_HELPER("NPC Controller", "npccontroller");

	using Component::Component;

	void PrePhysicsUpdate() override;

private:
	float myMaxDistance = 800.f;
};
