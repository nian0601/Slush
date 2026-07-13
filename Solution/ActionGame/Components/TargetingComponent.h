#pragma once

#include "EntitySystem/Component.h"
#include "EntitySystem/EntityHandle.h"
#include "EntitySystem/Entity.h"

class TargetingComponent : public Slush::Component
{
public:
	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		int myTargetType;
	};

public:
	COMPONENT_HELPER("Targeting", "targeting");

	TargetingComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);

	void Update() override;
	Slush::EntityHandle GetTarget() const { return myTarget; }

private:
	Slush::EntityHandle myTarget;
};
