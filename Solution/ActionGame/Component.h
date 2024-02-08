#pragma once

class Entity;
class Component
{
public:
	Component(Entity& anEntity)
		: myEntity(anEntity)
	{}
	virtual ~Component() {}

protected:
	Entity& myEntity;
};
