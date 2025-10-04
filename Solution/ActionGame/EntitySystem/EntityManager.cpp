#include "stdafx.h"

#include "EntityManager.h"
#include "EntityPrefab.h"

#include "ComponentRegistry.h"

#include "FW_TypeID.h"

#include "Components/AnimationComponent.h"
#include "Components/ExperienceComponent.h"
#include "Components/HealthComponent.h"
#include "Components/NPCControllerComponent.h"
#include "Components/PhysicsComponent.h"
#include "Components/PickupComponent.h"
#include "Components/PlayerControllerComponent.h"
#include "Components/ProjectileShootingComponent.h"
#include "Components/RemoveOnCollisionComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/StatsComponent.h"
#include "Components/TargetingComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/DamageDealerComponent.h"
#include "Components/HealthBarComponent.h"
#include "Components/TimedRemovalComponent.h"
#include "Components/DropComponent.h"


void EntityManager::RegisterComponents()
{
	ComponentRegistry& registry = ComponentRegistry::GetInstance();
	registry.RegisterComponent<SpriteComponent, SpriteComponent::Data>();
	registry.RegisterComponent<AnimationComponent>();
	registry.RegisterComponent<ProjectileShootingComponent, ProjectileShootingComponent::Data>();
	registry.RegisterComponent<HealthComponent, HealthComponent::Data>();
	registry.RegisterComponent<PlayerControllerComponent>();
	registry.RegisterComponent<NPCControllerComponent, NPCControllerComponent::Data>();
	registry.RegisterComponent<PhysicsComponent, PhysicsComponent::Data>();
	registry.RegisterComponent<RemoveOnCollisionComponent>();
	registry.RegisterComponent<TargetingComponent, TargetingComponent::Data>();
	registry.RegisterComponent<WeaponComponent, WeaponComponent::Data>();
	registry.RegisterComponent<ExperienceComponent>();
	registry.RegisterComponent<PickupComponent, PickupComponent::Data>();
	registry.RegisterComponent<StatsComponent, StatsComponent::Data>();
	registry.RegisterComponent<DamageDealerComponent, DamageDealerComponent::Data>();
	registry.RegisterComponent<HealthBarComponent>();
	registry.RegisterComponent<TimedRemovalComponent, TimedRemovalComponent::Data>();
	registry.RegisterComponent<DropComponent, DropComponent::Data>();
}

EntityManager::EntityManager(Slush::AssetStorage<EntityPrefab>& aPrefabStorage)
	: myPrefabStorage(aPrefabStorage)
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
