#include "stdafx.h"

#include <EntitySystem/EntityManager.h>
#include <EntitySystem/ComponentRegistry.h>

#include "Components/AnimationComponent.h"
#include "Components/CharacterAnimationComponent.h"
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


void Slush::EntityManager::RegisterComponents()
{
	Slush::ComponentRegistry& registry = Slush::ComponentRegistry::GetInstance();
	registry.RegisterComponent<SpriteComponent, SpriteComponent::Data>();
	registry.RegisterComponent<AnimationComponent>();
	registry.RegisterComponent<ProjectileShootingComponent, ProjectileShootingComponent::Data>();
	registry.RegisterComponent<HealthComponent, HealthComponent::Data>();
	registry.RegisterComponent<PlayerControllerComponent>();
	registry.RegisterComponent<NPCControllerComponent, NPCControllerComponent::Data>();
	registry.RegisterComponent<PhysicsComponent, PhysicsComponent::Data>();
	registry.RegisterComponent<RemoveOnCollisionComponent, RemoveOnCollisionComponent::Data>();
	registry.RegisterComponent<TargetingComponent, TargetingComponent::Data>();
	registry.RegisterComponent<WeaponComponent, WeaponComponent::Data>();
	registry.RegisterComponent<ExperienceComponent>();
	registry.RegisterComponent<PickupComponent, PickupComponent::Data>();
	registry.RegisterComponent<StatsComponent, StatsComponent::Data>();
	registry.RegisterComponent<DamageDealerComponent, DamageDealerComponent::Data>();
	registry.RegisterComponent<HealthBarComponent>();
	registry.RegisterComponent<TimedRemovalComponent, TimedRemovalComponent::Data>();
	registry.RegisterComponent<DropComponent, DropComponent::Data>();
	registry.RegisterComponent<CharacterAnimationComponent, CharacterAnimationComponent::Data>();
}
