#include "stdafx.h"

#include "EntitySystem/Components/AnimationComponent.h"
#include "HealthComponent.h"
#include "HealthBarComponent.h"

#include "EntitySystem/EntityComponentEvents.h"
#include <Graphics/RectSprite.h>

void HealthComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseIntField("maxhealth", myMaxHealth);
	aComponentHandle.ParseFloatField("graceperiodduration", myGracePeriodDuration);
}

void HealthComponent::Data::OnBuildUI()
{
	ImGui::InputInt("Max Health", &myMaxHealth);
	ImGui::InputFloat("Grace Period Duration", &myGracePeriodDuration);
}

//////////////////////////////////////////////////////////////////////////

HealthComponent::HealthComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
	const Data& healthData = anEntityPrefab.GetComponentData<HealthComponent>();

	SetMaxHealth(healthData.myMaxHealth);

	if (healthData.myGracePeriodDuration > 0.f)
		myGracePeriodTimer.Start(0.f);
}

void HealthComponent::OnEnterWorld()
{
	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
	myDamageAnimation = assets.GetAsset<Slush::Animation>("Blink");
}

void HealthComponent::SetMaxHealth(int aHealth)
{
	myMaxHealth = aHealth;
	myCurrentHealth = myMaxHealth;
}

void HealthComponent::DealDamage(int aDamageAmount)
{
	if (IsDead())
		return;

	if (myGracePeriodTimer.IsStarted())
	{
		if (!myGracePeriodTimer.HasExpired())
			return;

		myGracePeriodTimer.Start(myEntityPrefab.GetComponentData<HealthComponent>().myGracePeriodDuration);
	}

	EntityDamageTakenEvent damageTaken;
	damageTaken.myDamageAmount = aDamageAmount;
	damageTaken.myOldHealth = myCurrentHealth;
	damageTaken.myMaxHealth = myMaxHealth;

	myCurrentHealth -= aDamageAmount;
	myCurrentHealth = FW_Max(myCurrentHealth, 0);

	damageTaken.myNewHealth = myCurrentHealth;

	if (Slush::AnimationComponent* anim = myEntity.GetComponent<Slush::AnimationComponent>())
	{
		Slush::AnimationRuntime* animData = anim->PlayAnimation(*myDamageAnimation);
		animData->myEndColor = 0xFFFF0000;
	}

	if (HealthBarComponent* healthBar = myEntity.GetComponent<HealthBarComponent>())
		healthBar->OnDamageTaken(damageTaken);

	if (IsDead())
	{
		myEntity.OnDeath();
		myEntity.myIsMarkedForRemoval = true;
	}
}

void HealthComponent::RestoreHealth(int aHealAmount)
{
	if (IsDead())
		return;

	// TODO:
	// Change this either to a generic EntityHealthChange-event
	// Or split into DamageTaken vs HealthRestored events
	EntityDamageTakenEvent damageTaken;
	damageTaken.myDamageAmount = -aHealAmount;
	damageTaken.myOldHealth = myCurrentHealth;
	damageTaken.myMaxHealth = myMaxHealth;

	myCurrentHealth += aHealAmount;
	myCurrentHealth = FW_Min(myCurrentHealth, myMaxHealth);

	damageTaken.myNewHealth = myCurrentHealth;

	if (HealthBarComponent* healthBar = myEntity.GetComponent<HealthBarComponent>())
		healthBar->OnDamageTaken(damageTaken);
}

