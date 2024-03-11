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

EntityManager::EntityManager(Slush::AssetStorage<EntityPrefab>& aPrefabStorage)
	: myPrefabStorage(aPrefabStorage)
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

	Entity* entity = new Entity();
	proxy->myObject = entity;

	entity->myHandle = EntityHandle(proxy);

	myAddQueue.Add(entity);
	
	return entity;
}

Entity* EntityManager::CreateEntity(const Vector2f& aPosition, const EntityPrefab& aPrefab, Slush::PhysicsWorld& aPhysicsWorld, ProjectileManager& aProjectileManager)
{
	Entity* entity = CreateEntity();
	entity->myType = aPrefab.myEntityType;
	entity->myPosition = aPosition;

	if (aPrefab.mySprite.myEnabled)
	{
		entity->mySpriteComponent = new SpriteComponent(*entity);
		if (aPrefab.mySprite.myRadius > 0.f)
			entity->mySpriteComponent->MakeCircle(aPrefab.mySprite.myRadius, aPrefab.mySprite.myColor);
		else
			entity->mySpriteComponent->MakeRect(aPrefab.mySprite.mySize.x, aPrefab.mySprite.mySize.y, aPrefab.mySprite.myColor);
	}

	if (aPrefab.myAnimation.myEnabled)
		entity->myAnimationComponent = new AnimationComponent(*entity);

	if (aPrefab.myProjectileShooting.myEnabled)
	{
		entity->myProjectileShootingComponent = new ProjectileShootingComponent(*entity, aProjectileManager);
		entity->myProjectileShootingComponent->SetCooldown(aPrefab.myProjectileShooting.myCooldown);
	}

	if (aPrefab.myHealth.myEnabled)
	{
		entity->myHealthComponent = new HealthComponent(*entity);
		entity->myHealthComponent->SetMaxHealth(aPrefab.myHealth.myMaxHealth);
	}

	if (aPrefab.myPlayerController.myEnabled)
		entity->myPlayerControllerComponent = new PlayerControllerComponent(*entity);

	if (aPrefab.myNPCController.myEnabled)
		entity->myNPCControllerComponent = new NPCControllerComponent(*entity);

	if (aPrefab.myPhysics.myEnabled)
	{
		Slush::PhysicsShape* shape = nullptr;
		if (aPrefab.myPhysics.myMatchSprite)
		{
			if (aPrefab.mySprite.myEnabled)
			{
				if (aPrefab.mySprite.myRadius > 0.f)
					shape = new Slush::CircleShape(aPrefab.mySprite.myRadius);
				else
					shape = new Slush::AABBShape(aPrefab.mySprite.mySize);
			}
			else
			{
				SLUSH_ERROR("Failed to match Physics to Sprite since EntityPrefab doesnt have Sprite enabled, creating a Unit-circle as default");
				shape = new Slush::CircleShape(1.f);
			}
		}
		else
		{
			if (aPrefab.myPhysics.myRadius > 0.f)
				shape = new Slush::CircleShape(aPrefab.myPhysics.myRadius);
			else
				shape = new Slush::AABBShape(aPrefab.myPhysics.mySize);
		}

		if (shape)
		{
			entity->myPhysicsComponent = new PhysicsComponent(*entity, aPhysicsWorld);
			entity->myPhysicsComponent->myObject = new Slush::PhysicsObject(shape);
			entity->myPhysicsComponent->myObject->SetPosition(entity->myPosition);
			entity->myPhysicsComponent->myObject->myUserData.Set(entity->myPhysicsComponent);
			aPhysicsWorld.AddObject(entity->myPhysicsComponent->myObject);

			if (aPrefab.myPhysics.myStatic)
				entity->myPhysicsComponent->myObject->MakeStatic();
		}
		else
		{
			SLUSH_ERROR("Failed to create PhysicsShape, wont create PhysicsComponent for the entity");
		}
	}

	if (aPrefab.myRemoveOnCollision.myEnabled)
		entity->myRemoveOnCollisionComponent = new RemoveOnCollisionComponent(*entity);

	return entity;
}

Entity* EntityManager::CreateEntity(const Vector2f& aPosition, const char* aPrefabName, Slush::PhysicsWorld& aPhysicsWorld, ProjectileManager& aProjectileManager)
{
	if (const EntityPrefab* prefab = myPrefabStorage.GetAsset(aPrefabName))
		return CreateEntity(aPosition, *prefab, aPhysicsWorld, aProjectileManager);
	
	SLUSH_ERROR("Found no EntityPrefab called %s, creating a empty entity", aPrefabName);
	return CreateEntity();
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
