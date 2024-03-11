#pragma once

#include <FW_GrowingArray.h>

#include "EntityHandle.h"
#include <FW_Vector2.h>
#include <Core\AssetStorage.h>

class Entity;
class EntityPrefab;
class ProjectileManager;

namespace Slush
{
	class PhysicsWorld;
};

class EntityManager
{
public:
	EntityManager(Slush::AssetStorage<EntityPrefab>& aPrefabStorage);
	~EntityManager();

	Entity* CreateEntity();
	Entity* CreateEntity(const Vector2f& aPosition, const EntityPrefab& aPrefab, Slush::PhysicsWorld& aPhysicsWorld, ProjectileManager& aProjectileManager);
	Entity* CreateEntity(const Vector2f& aPosition, const char* aPrefabName, Slush::PhysicsWorld& aPhysicsWorld, ProjectileManager& aProjectileManager);

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
