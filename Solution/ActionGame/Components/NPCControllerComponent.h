#pragma once

#include "Component.h"

class NPCControllerComponent : public Component
{
public:
	struct Data : public Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		float myMovementSpeed = 100.f;
		float myMaxShootingDistance = 800.f;
	};

	COMPONENT_HELPER("NPC Controller", "npccontroller");

	NPCControllerComponent(Entity& aEntity, const EntityPrefab& anEntityPrefab);

	void OnEnterWorld() override;
	void PrePhysicsUpdate() override;
	void Update() override;

private:
	bool myHasFinishedSpawning = false;
	const Data& myData;
	float myMaxDistance = 800.f;
};
