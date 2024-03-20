#include "EntityManager.h"
#include "Entity.h"
#include "EntityPrefab.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "ProjectileShootingComponent.h"
#include "HealthComponent.h"
#include "PlayerControllerComponent.h"
#include "NPCControllerComponent.h"
#include "PhysicsComponent.h"
#include <Physics\PhysicsWorld.h>
#include <Core\Log.h>
#include "RemoveOnCollisionComponent.h"
#include "TargetingComponent.h"

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

Entity* EntityManager::CreateEntity()
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
	Entity* entity = CreateEntity();
	entity->myType = static_cast<Entity::Type>(aPrefab.myEntityType);
	entity->myPosition = aPosition;

	if (aPrefab.mySprite.myEnabled)
		entity->mySpriteComponent = new SpriteComponent(*entity, aPrefab);

	if (aPrefab.myAnimation.myEnabled)
		entity->myAnimationComponent = new AnimationComponent(*entity, aPrefab);

	if (aPrefab.myProjectileShooting.myEnabled)
		entity->myProjectileShootingComponent = new ProjectileShootingComponent(*entity, aPrefab);

	if (aPrefab.myHealth.myEnabled)
		entity->myHealthComponent = new HealthComponent(*entity, aPrefab);

	if (aPrefab.myPlayerController.myEnabled)
		entity->myPlayerControllerComponent = new PlayerControllerComponent(*entity, aPrefab);

	if (aPrefab.myNPCController.myEnabled)
		entity->myNPCControllerComponent = new NPCControllerComponent(*entity, aPrefab);

	if (aPrefab.myPhysics.myEnabled)
		entity->myPhysicsComponent = new PhysicsComponent(*entity, aPrefab, myPhysicsWorld);

	if (aPrefab.myRemoveOnCollision.myEnabled)
		entity->myRemoveOnCollisionComponent = new RemoveOnCollisionComponent(*entity, aPrefab);

	if (aPrefab.myTargeting.myEnabled)
		entity->myTargetingComponent = new TargetingComponent(*entity, aPrefab);

	return entity;
}

Entity* EntityManager::CreateEntity(const Vector2f& aPosition, const char* aPrefabName)
{
	if (const EntityPrefab* prefab = myPrefabStorage.GetAsset(aPrefabName))
		return CreateEntity(aPosition, *prefab);
	
	SLUSH_ERROR("Found no EntityPrefab called %s, creating a empty entity", aPrefabName);
	return CreateEntity();
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
