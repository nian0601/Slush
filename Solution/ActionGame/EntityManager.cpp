#include "stdafx.h"

#include "EntityManager.h"
#include "EntityPrefab.h"

#include "ComponentRegistry.h"

#include "FW_TypeID.h"

#include "Component.h"

#include "AnimationComponent.h"
#include "ExperienceComponent.h"
#include "HealthComponent.h"
#include "NPCControllerComponent.h"
#include "PhysicsComponent.h"
#include "PickupComponent.h"
#include "PlayerControllerComponent.h"
#include "ProjectileShootingComponent.h"
#include "RemoveOnCollisionComponent.h"
#include "SpriteComponent.h"
#include "StatsComponent.h"
#include "TargetingComponent.h"
#include "WeaponComponent.h"
#include "DamageDealerComponent.h"
#include "HealthBarComponent.h"


EntityManager::EntityManager(Slush::AssetStorage<EntityPrefab>& aPrefabStorage)
	: myPrefabStorage(aPrefabStorage)
{
	ComponentRegistry& registry = ComponentRegistry::GetInstance();
	registry.RegisterComponent<SpriteComponent, SpriteComponent::Data>();
	registry.RegisterComponent<AnimationComponent>();
	registry.RegisterComponent<ProjectileShootingComponent, ProjectileShootingComponent::Data>();
	registry.RegisterComponent<HealthComponent, HealthComponent::Data>();
	registry.RegisterComponent<PlayerControllerComponent>();
	registry.RegisterComponent<NPCControllerComponent>();
	registry.RegisterComponent<PhysicsComponent, PhysicsComponent::Data>();
	registry.RegisterComponent<RemoveOnCollisionComponent>();
	registry.RegisterComponent<TargetingComponent, TargetingComponent::Data>();
	registry.RegisterComponent<WeaponComponent, WeaponComponent::Data>();
	registry.RegisterComponent<ExperienceComponent>();
	registry.RegisterComponent<PickupComponent>();
	registry.RegisterComponent<StatsComponent, StatsComponent::Data>();
	registry.RegisterComponent<DamageDealerComponent, DamageDealerComponent::Data>();
	registry.RegisterComponent<HealthBarComponent>();
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
	entity->myType = static_cast<EntityType>(aPrefab.myEntityType);
	entity->myPosition = aPosition;
	entity->CreateComponents(aPrefab);

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

void EntityManager::FindEntitiesOfType(EntityType aType, FW_GrowingArray<EntityHandle>& outEntityHandles) const
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
