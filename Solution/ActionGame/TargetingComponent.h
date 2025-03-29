#pragma once

#include "Component.h"
#include "EntityHandle.h"
#include "Entity.h"

class EntityPrefab;

class TargetingComponent : public Component
{
public:
	struct Data : public Component::BaseData
	{
		Data();

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		int myTargetType;
	};

public:
	TargetingComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);

	void Update() override;
	EntityHandle GetTarget() const { return myTarget; }

private:
	EntityHandle myTarget;
};