#pragma once

#include "EntityComponentEvents.h"
#include <FW_TypeID.h>

class Entity;
class EntityPrefab;

class Component
{
public:
	Component(Entity& anEntity, const EntityPrefab& anEntityPrefab)
		: myEntity(anEntity)
		, myEntityPrefab(anEntityPrefab)
	{}
	virtual ~Component() {}

	virtual void PrePhysicsUpdate() {};
	virtual void Update() {};
	virtual void Render() {};

	virtual void OnCollision(Entity& aOtherEntity) { aOtherEntity; };
	virtual void OnDeath() {};

	Entity& myEntity;
	const EntityPrefab& myEntityPrefab;
};

template <typename ComponentType>
unsigned int GetComponentID()
{
	return FW_TypeID<Component>::GetID<ComponentType>();
}

#define MAX_COMPONENTS 32