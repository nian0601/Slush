#include "stdafx.h"

#include "AnimationComponent.h"
#include "HealthComponent.h"
#include "HealthBarComponent.h"
#include "EntityPrefab.h"

#include <Graphics/RectSprite.h>

HealthComponent::HealthComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	SetMaxHealth(anEntityPrefab.myHealth.myMaxHealth);

	if (anEntityPrefab.myHealth.myGracePeriodDuration > 0.f)
		myGracePeriodTimer.Start(0.f);
}

void HealthComponent::SetMaxHealth(int aHealth)
{
	myMaxHealth = aHealth;
	myCurrentHealth = myMaxHealth;
}

void HealthComponent::DealDamage(int aDamageAmount)
{
	if (myGracePeriodTimer.IsStarted())
	{
		if (!myGracePeriodTimer.HasExpired())
			return;

		myGracePeriodTimer.Start(myEntityPrefab.myHealth.myGracePeriodDuration);
	}

	EntityDamageTakenEvent damageTaken;
	damageTaken.myDamageAmount = aDamageAmount;
	damageTaken.myOldHealth = myCurrentHealth;
	damageTaken.myMaxHealth = myMaxHealth;

	myCurrentHealth -= aDamageAmount;
	myCurrentHealth = FW_Max(myCurrentHealth, 0);

	damageTaken.myNewHealth = myCurrentHealth;

	if (AnimationComponent* anim = myEntity.GetComponent<AnimationComponent>())
		anim->PlayBlink();

	if (HealthBarComponent* healthBar = myEntity.GetComponent<HealthBarComponent>())
		healthBar->OnDamageTaken(damageTaken);

	if (IsDead())
	{
		myEntity.OnDeath();
		myEntity.myIsMarkedForRemoval = true;
	}
}

