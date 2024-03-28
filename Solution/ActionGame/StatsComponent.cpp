#include "StatsComponent.h"

#include "Entity.h"
#include "EntityPrefab.h"

void StatsComponent::Stat::Upgrade(float aValue)
{
	myStatModifierValue += aValue;
	++myNumberOfUpgrades;
}

void StatsComponent::AddCooldownReductionUpgrade()
{
	myCooldownReduction.Upgrade(myEntityPrefab.myStats.myCooldownPerUpgrade);
}

void StatsComponent::AddDamageUpgrade()
{
	myCooldownReduction.Upgrade(myEntityPrefab.myStats.myDamagePerUpgrade);
}

bool StatsComponent::CanUpgradeCooldownReduction() const
{
	return myCooldownReduction.myNumberOfUpgrades < myEntityPrefab.myStats.myMaxCooldownUpgrades;
}

bool StatsComponent::CanUpgradeDamage() const
{
	return myDamageModifier.myNumberOfUpgrades < myEntityPrefab.myStats.myMaxDamageUpgrades;
}
