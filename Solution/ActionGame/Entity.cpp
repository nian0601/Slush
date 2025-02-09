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

	FW_SAFE_DELETE(myExperienceComponent);
	FW_SAFE_DELETE(myPickupComponent);
	FW_SAFE_DELETE(myStatsComponent);
	FW_SAFE_DELETE(myDamageDealerComponent);
	FW_SAFE_DELETE(myHealthBarComponent);
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

	if (myExperienceComponent)
		myExperienceComponent->Render();

	if (myHealthBarComponent)
		myHealthBarComponent->Render();
}

void Entity::OnCollision(Entity& aOtherEntity)
{
	for (Component* component : myPackedComponents)
		component->OnCollision(aOtherEntity);

	if (myPickupComponent)
		myPickupComponent->OnCollision(aOtherEntity);

	if (myDamageDealerComponent)
		myDamageDealerComponent->OnCollision(aOtherEntity);
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
	if (aPrefab.mySprite.myEnabled)
	{
		int index = FW_TypeID<Component>::GetID<SpriteComponent>();
		myComponents[index] = new SpriteComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.myAnimation.myEnabled)
	{
		int index = FW_TypeID<Component>::GetID<AnimationComponent>();
		myComponents[index] = new AnimationComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.myProjectileShooting.myEnabled)
	{
		int index = FW_TypeID<Component>::GetID<ProjectileShootingComponent>();
		myComponents[index] = new ProjectileShootingComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.myPlayerController.myEnabled)
	{
		int index = FW_TypeID<Component>::GetID<PlayerControllerComponent>();
		myComponents[index] = new PlayerControllerComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.myNPCController.myEnabled)
	{
		int index = FW_TypeID<Component>::GetID<NPCControllerComponent>();
		myComponents[index] = new NPCControllerComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.myHealth.myEnabled)
	{
		int index = FW_TypeID<Component>::GetID<HealthComponent>();
		myComponents[index] = new HealthComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.myPhysics.myEnabled)
	{
		int index = FW_TypeID<Component>::GetID<PhysicsComponent>();
		myComponents[index] = new PhysicsComponent(*this, aPrefab, aPhysicsWorld);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.myRemoveOnCollision.myEnabled)
	{
		int index = FW_TypeID<Component>::GetID<RemoveOnCollisionComponent>();
		myComponents[index] = new RemoveOnCollisionComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.myTargeting.myEnabled)
	{
		int index = FW_TypeID<Component>::GetID<TargetingComponent>();
		myComponents[index] = new TargetingComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.myWeaponComponent.myEnabled)
	{
		int index = FW_TypeID<Component>::GetID<WeaponComponent>();
		myComponents[index] = new WeaponComponent(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}

	if (aPrefab.myExperience.myEnabled)
		myExperienceComponent = new ExperienceComponent(*this, aPrefab);

	if (aPrefab.myPickup.myEnabled)
		myPickupComponent = new PickupComponent(*this, aPrefab);

	if (aPrefab.myStats.myEnabled)
		myStatsComponent = new StatsComponent(*this, aPrefab);

	if (aPrefab.myDamageDealer.myEnabled)
		myDamageDealerComponent = new DamageDealerComponent(*this, aPrefab);

	if (aPrefab.myHealthBar.myEnabled)
		myHealthBarComponent = new HealthBarComponent(*this, aPrefab);
}