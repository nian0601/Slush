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

	Entity& myEntity;
	const EntityPrefab& myEntityPrefab;
};
