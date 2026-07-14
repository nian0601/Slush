#include "stdafx.h"

#include "EntityManager.h"
#include "EntityPrefab.h"

#include "FW_TypeID.h"

namespace Slush
{
	EntityManager::EntityManager()
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
		entity->myPosition = aPosition;
		entity->CreateComponents(aPrefab);

		return entity;
	}

	Entity* EntityManager::CreateEntity(const Vector2f& aPosition, const char* aPrefabName)
	{
		AssetRegistry& assets = AssetRegistry::GetInstance();
		if (const EntityPrefab* prefab = assets.GetAsset<EntityPrefab>(aPrefabName))
			return CreateEntity(aPosition, *prefab);

		SLUSH_ERROR("Found no EntityPrefab called %s, creating a empty entity", aPrefabName);
		return CreateEmptyEntity();
	}

	Entity* EntityManager::CreateEntity(const Vector2f& aPosition, const FW_String& aPrefabName)
	{
		return CreateEntity(aPosition, aPrefabName.GetBuffer());
	}

	void EntityManager::DeleteAllEntities()
	{
		for (Entity* entity : myEntities)
			entity->myHandle.myProxy->myObject = nullptr;

		myEntities.DeleteAll();
	}

	void EntityManager::GetAllEntities(FW_GrowingArray<EntityHandle>& outEntityHandles) const
	{
		for (Entity* entity : myEntities)
			outEntityHandles.Add(entity->myHandle);
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
		{
			entity->OnEnterWorld();
			myEntities.Add(entity);
		}

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
}
