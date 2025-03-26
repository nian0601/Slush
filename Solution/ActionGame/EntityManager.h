#pragma once

#include <FW_GrowingArray.h>

#include "EntityHandle.h"
#include "Entity.h"
#include <FW_Vector2.h>
#include <Core/Assets/AssetStorage.h>

class Entity;
class EntityPrefab;

namespace Slush
{
	class PhysicsWorld;
};

class EntityManager
{
public:
	EntityManager(Slush::AssetStorage<EntityPrefab>& aPrefabStorage);
	~EntityManager();

	Entity* CreateEmptyEntity();
	Entity* CreateEntity(const Vector2f& aPosition, const EntityPrefab& aPrefab);
	Entity* CreateEntity(const Vector2f& aPosition, const char* aPrefabName);

	void DeleteAllEntities();

	void FindEntitiesOfType(EntityType aType, FW_GrowingArray<EntityHandle>& outEntityHandles) const;

	void PrePhysicsUpdate();

	void Update();
	void Render();

	void EndFrame();

private:
	void CleanupProxyStorage();

	FW_GrowingArray<EntityHandle::ProxyObject*> myProxyStorage;
	FW_GrowingArray<Entity*> myEntities;

	FW_GrowingArray<Entity*> myAddQueue;

	Slush::AssetStorage<EntityPrefab>& myPrefabStorage;
};
