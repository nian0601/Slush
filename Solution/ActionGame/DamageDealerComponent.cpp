#include "stdafx.h"

#include "DamageDealerComponent.h"
#include "EntityPrefab.h"
#include "HealthComponent.h"

DamageDealerComponent::DamageDealerComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	myDamage = myEntityPrefab.myDamageDealer.myDamage;
}

void DamageDealerComponent::OnCollision(Entity& aOtherEntity)
{
	Entity::Type otherType = aOtherEntity.myType;
	if (otherType == Entity::ENVIRONMENT)
		return;

	HealthComponent* otherHealth = aOtherEntity.GetComponent<HealthComponent>();
	if (!otherHealth)
		return;

	if (myEntity.IsPlayerOwned() && aOtherEntity.IsNPCOwned())
		otherHealth->DealDamage(myDamage);
	else if (myEntity.IsNPCOwned() && aOtherEntity.IsPlayerOwned())
		otherHealth->DealDamage(myDamage);
}