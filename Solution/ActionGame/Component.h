#pragma once

#include "EntityComponentEvents.h"

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
