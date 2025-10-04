#include "stdafx.h"

#include "DamageDealerComponent.h"

#include "HealthComponent.h"

void DamageDealerComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseIntField("damage", myDamage);
}

void DamageDealerComponent::Data::OnBuildUI()
{
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("Damage", &myDamage);
}

//////////////////////////////////////////////////////////////////////////

DamageDealerComponent::DamageDealerComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	myDamage = myEntityPrefab.GetDamageDealerData().myDamage;
}

void DamageDealerComponent::OnCollision(Entity& aOtherEntity)
{
	EntityType otherType = aOtherEntity.myType;
	if (otherType == EntityType::ENVIRONMENT)
		return;

	HealthComponent* otherHealth = aOtherEntity.GetComponent<HealthComponent>();
	if (!otherHealth)
		return;

	if (myEntity.IsPlayerOwned() && aOtherEntity.IsNPCOwned())
		otherHealth->DealDamage(myDamage);
	else if (myEntity.IsNPCOwned() && aOtherEntity.IsPlayerOwned())
		otherHealth->DealDamage(myDamage);
}