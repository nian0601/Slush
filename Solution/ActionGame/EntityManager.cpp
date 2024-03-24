#include "EntityManager.h"
#include "Entity.h"
#include "EntityPrefab.h"

#include <Core\Log.h>

EntityManager::EntityManager(Slush::AssetStorage<EntityPrefab>& aPrefabStorage, Slush::PhysicsWorld& aPhysicsWorld)
	: myPrefabStorage(aPrefabStorage)
	, myPhysicsWorld(aPhysicsWorld)
{
}

EntityManager::~EntityManager()
{
	myProxyStorage.DeleteAll();
	myEntities.DeleteAll();
}

Entity* EntityManager::CreateEmptyEntity()
{
	EntityHandle::ProxyObject* proxy = new EntityHandle::ProxyObject();
	myProxyStorage.Add(proxy);

	Entity* entity = new Entity(*this);
	proxy->myObject = entity;

	entity->myHandle = EntityHandle(proxy);

	myAddQueue.Add(entity);
	
	return entity;
}

Entity* EntityManager::CreateEntity(const Vector2f& aPosition, const EntityPrefab& aPrefab)
{
	Entity* entity = CreateEmptyEntity();
	entity->myType = static_cast<Entity::Type>(aPrefab.myEntityType);
	entity->myPosition = aPosition;
	entity->CreateComponents(aPrefab, myPhysicsWorld);

	return entity;
}

Entity* EntityManager::CreateEntity(const Vector2f& aPosition, const char* aPrefabName)
{
	if (const EntityPrefab* prefab = myPrefabStorage.GetAsset(aPrefabName))
		return CreateEntity(aPosition, *prefab);
	
	SLUSH_ERROR("Found no EntityPrefab called %s, creating a empty entity", aPrefabName);
	return CreateEmptyEntity();
}

void EntityManager::DeleteAllEntities()
{
	for (Entity* entity : myEntities)
		entity->myHandle.myProxy->myObject = nullptr;

	myEntities.DeleteAll();
}

void EntityManager::FindEntitiesOfType(Entity::Type aType, FW_GrowingArray<EntityHandle>& outEntityHandles) const
{
	for (Entity* entity : myEntities)
	{
		if (entity->myType == aType)
			outEntityHandles.Add(entity->myHandle);
	}
}

void EntityManager::PrePhysicsUpdate()
{
	for (Entity* entity : myEntities)
		entity->PrePhysicsUpdate();
}

void EntityManager::Update()
{
	for (Entity* entity : myEntities)
		entity->Update();
}

void EntityManager::Render()
{
	for (Entity* entity : myEntities)
		entity->Render();
}

void EntityManager::EndFrame()
{
	for (int i = 0; i < myEntities.Count();)
	{
		Entity* entity = myEntities[i];
		if (!entity->myIsMarkedForRemoval)
		{
			++i;
			continue;
		}

		bool removedProxy = false;
		for (EntityHandle::ProxyObject* proxy : myProxyStorage)
		{
			if (proxy->myObject == entity)
			{
				proxy->myObject = nullptr;
				myEntities.DeleteCyclicAtIndex(i);
				removedProxy = true;
			}
		}

		FW_ASSERT(removedProxy, "Didnt find EntityProxy for an Entity that is marked for removal");
	}

	for (Entity* entity : myAddQueue)
		myEntities.Add(entity);

	myAddQueue.RemoveAll();

	CleanupProxyStorage();
}

void EntityManager::CleanupProxyStorage()
{
	for (int i = 0; i < myProxyStorage.Count();)
	{
		if (myProxyStorage[i]->myRefCount < 1)
		{
			myProxyStorage.DeleteCyclicAtIndex(i);
		}
		else
		{
			++i;
		}
	}
}
