#pragma once

#include <EntitySystem/Component.h>

class TargetableComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Targetable", "targetable", 1);

	TargetableComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);
};
