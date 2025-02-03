#pragma once

#include "Component.h"

class StatsComponent : public Component
{
public:
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
