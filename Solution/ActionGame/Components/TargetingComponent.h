#pragma once

#include "EntitySystem/Component.h"
#include "EntitySystem/EntityHandle.h"
#include "EntitySystem/Entity.h"
#include "EntitySystem/Components/PhysicsComponent.h"
#include "CollisionFlag.h"

class TargetingComponent : public Slush::Component
{
public:
	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
		void OnBuildUI() override;

		int myTargetFlag = CollisionUtils::COL_ENVIRONMENT;
	};

public:
	COMPONENT_HELPER("Targeting", "targeting", 1);

	TargetingComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);

	void Update() override;
	Slush::EntityHandle GetTarget() const { return myTarget; }

private:
	Slush::EntityHandle myTarget;
};
