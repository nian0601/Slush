#include "stdafx.h"

#include "EntityPrefab.h"
#include "StatsComponent.h"

void StatsComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseIntField("maxcooldownupgrades", myMaxCooldownUpgrades);
	aComponentHandle.ParseFloatField("cooldownperupgrade", myCooldownPerUpgrade);

	aComponentHandle.ParseIntField("maxdamageupgrades", myMaxDamageUpgrades);
	aComponentHandle.ParseFloatField("damageperupgrade", myDamagePerUpgrade);

	aComponentHandle.ParseIntField("maxadditionalprojectileupgrades", myMaxAdditionalProjectileUpgrades);
	aComponentHandle.ParseFloatField("additionalprojectilesperupgrade", myAdditionalProjectilesPerUpgrade);
}

void StatsComponent::Data::OnBuildUI()
{
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("Max Cooldown Upgrades", &myMaxCooldownUpgrades);

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Cooldown Per Upgrade", &myCooldownPerUpgrade, 0.05f, 1.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("Max Damage Upgrades", &myMaxDamageUpgrades);

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Damage Per Upgrade", &myDamagePerUpgrade, 0.05f, 1.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("Max Additional Projectiles Upgrades", &myMaxAdditionalProjectileUpgrades);

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Additional Projectiles Per Upgrade", &myAdditionalProjectilesPerUpgrade, 1.f, 1.f, "%.0f");
}

//////////////////////////////////////////////////////////////////////////

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