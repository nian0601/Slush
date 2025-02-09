#include "stdafx.h"

#include "Entity.h"

#include "AnimationComponent.h"
#include "EntityManager.h"
#include "EntityPrefab.h"
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
#include <FW_TypeID.h>

Entity::Entity(EntityManager& aEntityManager)
	: myEntityManager(aEntityManager)
{
	myComponents.Fill(nullptr);
}

Entity::~Entity()
{
	myComponents.DeleteAll();
}

void Entity::PrePhysicsUpdate()
{
	for (Component* component : myPackedComponents)
		component->PrePhysicsUpdate();
}

void Entity::Update()
{
	for (Component* component : myPackedComponents)
		component->Update();
}

void Entity::Render()
{
	for (Component* component : myPackedComponents)
		component->Render();
}

void Entity::OnCollision(Entity& aOtherEntity)
{
	for (Component* component : myPackedComponents)
		component->OnCollision(aOtherEntity);
}

void Entity::OnDeath()
{
	for (Component* component : myPackedComponents)
		component->OnDeath();

	if (NPCControllerComponent* npcController = GetComponent<NPCControllerComponent>())
		myEntityManager.CreateEntity(myPosition, "ExpPickup");
}

void Entity::CreateComponents(const EntityPrefab& aPrefab, Slush::PhysicsWorld& aPhysicsWorld)
{
	if (aPrefab.Has<SpriteComponent>())
	{
		int index = FW_TypeID<Component>::GetID<SpriteComponent>();
		myComponents[index] = new SpriteComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<AnimationComponent>())
	{
		int index = FW_TypeID<Component>::GetID<AnimationComponent>();
		myComponents[index] = new AnimationComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<ProjectileShootingComponent>())
	{
		int index = FW_TypeID<Component>::GetID<ProjectileShootingComponent>();
		myComponents[index] = new ProjectileShootingComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<PlayerControllerComponent>())
	{
		int index = FW_TypeID<Component>::GetID<PlayerControllerComponent>();
		myComponents[index] = new PlayerControllerComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<NPCControllerComponent>())
	{
		int index = FW_TypeID<Component>::GetID<NPCControllerComponent>();
		myComponents[index] = new NPCControllerComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<HealthComponent>())
	{
		int index = FW_TypeID<Component>::GetID<HealthComponent>();
		myComponents[index] = new HealthComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<PhysicsComponent>())
	{
		int index = FW_TypeID<Component>::GetID<PhysicsComponent>();
		myComponents[index] = new PhysicsComponent(*this, aPrefab, aPhysicsWorld);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<RemoveOnCollisionComponent>())
	{
		int index = FW_TypeID<Component>::GetID<RemoveOnCollisionComponent>();
		myComponents[index] = new RemoveOnCollisionComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<TargetingComponent>())
	{
		int index = FW_TypeID<Component>::GetID<TargetingComponent>();
		myComponents[index] = new TargetingComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<WeaponComponent>())
	{
		int index = FW_TypeID<Component>::GetID<WeaponComponent>();
		myComponents[index] = new WeaponComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<ExperienceComponent>())
	{
		int index = FW_TypeID<Component>::GetID<ExperienceComponent>();
		myComponents[index] = new ExperienceComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<PickupComponent>())
	{
		int index = FW_TypeID<Component>::GetID<PickupComponent>();
		myComponents[index] = new PickupComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<StatsComponent>())
	{
		int index = FW_TypeID<Component>::GetID<StatsComponent>();
		myComponents[index] = new StatsComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<DamageDealerComponent>())
	{
		int index = FW_TypeID<Component>::GetID<DamageDealerComponent>();
		myComponents[index] = new DamageDealerComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.Has<HealthBarComponent>())
	{
		int index = FW_TypeID<Component>::GetID<HealthBarComponent>();
		myComponents[index] = new HealthBarComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}
}