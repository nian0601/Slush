#pragma once

#include <FW_GrowingArray.h>

#include "EntityHandle.h"

class Entity;
class EntityManager
{
public:
	~EntityManager();

	Entity* CreateEntity();

	void PrePhysicsUpdate();

	void Update();
	void Render();

	void EndFrame();

private:
	void CleanupProxyStorage();

	FW_GrowingArray<EntityHandle::ProxyObject*> myProxyStorage;
	FW_GrowingArray<Entity*> myEntities;
};
