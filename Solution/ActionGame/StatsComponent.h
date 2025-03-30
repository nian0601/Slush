#pragma once

#include "Component.h"

class StatsComponent : public Component
{
public:
	struct Data : public Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		int myMaxCooldownUpgrades = 5;
		float myCooldownPerUpgrade = 0.2f;

		int myMaxDamageUpgrades = 10;
		float myDamagePerUpgrade = 1.f;

		int myMaxAdditionalProjectileUpgrades = 3;
		float myAdditionalProjectilesPerUpgrade = 1.f;
	};

public:
	COMPONENT_HELPER("Stats", "stats");

	using Component::Component;

	void AddCooldownReductionUpgrade();
	void AddDamageUpgrade();
	void AddAdditionalProjectilesUpgrade();

	bool CanUpgradeCooldownReduction() const;
	bool CanUpgradeDamage() const;
	bool CanUpgradeAdditionalProjectiles() const;

	float GetCooldownReduction() const { return myCooldownReduction.myStatModifierValue; }
	float GetDamageModifier() const { return myDamageModifier.myStatModifierValue; }
	int GetAdditionalProjectiles() const { return static_cast<int>(myAdditionalProjectilesModifier.myStatModifierValue); }

private:
	struct Stat
	{
		void Upgrade(float aValue);

		float myStatModifierValue = 1.f;
		int myNumberOfUpgrades = 0;
	};

	Stat myCooldownReduction;
	Stat myDamageModifier;
	Stat myAdditionalProjectilesModifier;
};
