#pragma once

#include "Component.h"
#include "EntityHandle.h"

class NPCControllerComponent : public Component
{
public:
	COMPONENT_HELPER("NPC Controller", "npccontroller");

	using Component::Component;

	void OnEnterWorld() override;
	void PrePhysicsUpdate() override;
	void Update() override;

private:
	bool myHasFinishedSpawning = false;
	float myMaxDistance = 800.f;
};
