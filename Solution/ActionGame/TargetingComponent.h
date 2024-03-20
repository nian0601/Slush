#pragma once

#include "Component.h"
#include "EntityHandle.h"

class EntityPrefab;

class TargetingComponent : public Component
{
public:
	TargetingComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);

	void Update();
	EntityHandle GetTarget() const { return myTarget; }

private:
	EntityHandle myTarget;
};