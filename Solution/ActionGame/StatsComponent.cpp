#include "stdafx.h"

#include "EntityPrefab.h"
#include "StatsComponent.h"

void StatsComponent::Stat::Upgrade(float aValue)
{
	myStatModifierValue += aValue;
	++myNumberOfUpgrades;
}

void StatsComponent::AddCooldownReductionUpgrade()
{
	myCooldownReduction.Upgrade(myEntityPrefab.GetStatsData().myCooldownPerUpgrade);
}

void StatsComponent::AddDamageUpgrade()
{
	myDamageModifier.Upgrade(myEntityPrefab.GetStatsData().myDamagePerUpgrade);
}

void StatsComponent::AddAdditionalProjectilesUpgrade()
{
	myAdditionalProjectilesModifier.Upgrade(myEntityPrefab.GetStatsData().myAdditionalProjectilesPerUpgrade);
}

bool StatsComponent::CanUpgradeCooldownReduction() const
{
	return myCooldownReduction.myNumberOfUpgrades < myEntityPrefab.GetStatsData().myMaxCooldownUpgrades;
}

bool StatsComponent::CanUpgradeDamage() const
{
	return myDamageModifier.myNumberOfUpgrades < myEntityPrefab.GetStatsData().myMaxDamageUpgrades;
}

bool StatsComponent::CanUpgradeAdditionalProjectiles() const
{
	return myAdditionalProjectilesModifier.myNumberOfUpgrades < myEntityPrefab.GetStatsData().myMaxAdditionalProjectileUpgrades;
}
