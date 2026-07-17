#pragma once

#include "EntitySystem/Component.h"
#include "Core\Assets\AssetReference.h"

namespace Slush
{
	class Animation;
}

class NPCControllerComponent : public Slush::Component
{
public:
	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
		void OnBuildUI() override;
		void ResolveDependencies() override;

		float myMovementSpeed = 100.f;
		float myMaxShootingDistance = 800.f;
		Slush::AssetReference<Slush::Animation> mySpawnAnimationID;
	};

	COMPONENT_HELPER("NPC Controller", "npccontroller", 1);

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
