#pragma once

#include "EntitySystem/Component.h"

namespace Slush
{
	class Animation;
}

class NPCControllerComponent : public Slush::Component
{
public:
	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		float myMovementSpeed = 100.f;
		float myMaxShootingDistance = 800.f;
		FW_String mySpawnAnimationID;
	};

	COMPONENT_HELPER("NPC Controller", "npccontroller");

	NPCControllerComponent(Slush::Entity& aEntity, const Slush::EntityPrefab& anEntityPrefab);

	void OnEnterWorld() override;
	void PrePhysicsUpdate() override;
	void Update() override;

private:
	const Data& myData;
	bool myHasFinishedSpawning = false;
	float myMaxDistance = 800.f;
	const Slush::Animation* mySpawnAnimation = nullptr;
};
