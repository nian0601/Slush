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
	CreateComponent<SpriteComponent>(aPrefab);
	CreateComponent<AnimationComponent>(aPrefab);
	CreateComponent<ProjectileShootingComponent>(aPrefab);
	CreateComponent<PlayerControllerComponent>(aPrefab);
	CreateComponent<NPCControllerComponent>(aPrefab);
	CreateComponent<HealthComponent>(aPrefab);


	if (aPrefab.Has<PhysicsComponent>())
	{
		int index = GetComponentID<PhysicsComponent>();
		myComponents[index] = new PhysicsComponent(*this, aPrefab, aPhysicsWorld);
		myPackedComponents.Add(myComponents[index]);
	}

	CreateComponent<RemoveOnCollisionComponent>(aPrefab);
	CreateComponent<TargetingComponent>(aPrefab);
	CreateComponent<WeaponComponent>(aPrefab);
	CreateComponent<ExperienceComponent>(aPrefab);
	CreateComponent<PickupComponent>(aPrefab);
	CreateComponent<StatsComponent>(aPrefab);
	CreateComponent<DamageDealerComponent>(aPrefab);
	CreateComponent<HealthBarComponent>(aPrefab);
}