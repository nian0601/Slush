#include "stdafx.h"

#include "HealthComponent.h"

#include <EntitySystem/Entity.h>
#include <imgui/ImGuiWidgets.h>

void HealthComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseIntField("maxhealth", myMaxHealth);
}

void HealthComponent::Data::OnBuildUI()
{
	Slush::ImGuiWidgets::InputInt("Max Health", &myMaxHealth);
}

HealthComponent::HealthComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
	myCurrentHealth = anEntityPrefab.GetComponentData<HealthComponent>().myMaxHealth;
}

void HealthComponent::DealDamage(int aDamageAmount)
{
	if (IsDead())
		return;

	myCurrentHealth -= aDamageAmount;
	if (myCurrentHealth <= 0)
	{
		myCurrentHealth = 0;
		myEntity.OnDeath();
		myEntity.myIsMarkedForRemoval = true;
	}
}
