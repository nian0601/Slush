#include "stdafx.h"

#include "Entity.h"

#include "EntityManager.h"
#include "EntityPrefab.h"

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

#include <FW_TypeID.h>
#include "ComponentRegistry.h"

Entity::Entity(EntityManager& aEntityManager)
	: myEntityManager(aEntityManager)
{
	myComponents.Fill(nullptr);
}

Entity::~Entity()
{
	myComponents.DeleteAll();
}

void Entity::OnEnterWorld()
{
	for (Component* component : myPackedComponents)
		component->OnEnterWorld();
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
}

void Entity::CreateComponents(const EntityPrefab& aPrefab)
{
	for (const IComponentFactory* factory : ComponentRegistry::GetInstance().GetFactories())
	{
		if (aPrefab.Has(factory->GetID()))
		{
			int index = factory->GetID();
			myComponents[index] = factory->CreateComponent(*this, aPrefab);
			myPackedComponents.Add(myComponents[index]);
		}
	}
}