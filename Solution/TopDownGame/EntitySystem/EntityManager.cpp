#include "stdafx.h"

#include <EntitySystem/ComponentRegistry.h>
#include <EntitySystem/Components/SpriteComponent.h>
#include <EntitySystem/EntityManager.h>

#include "Components/MovementComponent.h"
#include "Components/HealthComponent.h"

void Slush::EntityManager::RegisterComponents()
{
	Slush::ComponentRegistry& registry = Slush::ComponentRegistry::GetInstance();
	registry.RegisterComponent<Slush::SpriteComponent, Slush::SpriteComponent::Data>();
	registry.RegisterComponent<MovementComponent, MovementComponent::Data>();
	registry.RegisterComponent<HealthComponent, HealthComponent::Data>();
}
