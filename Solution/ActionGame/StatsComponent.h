#pragma once

#include "Component.h"

class StatsComponent : public Component
{
public:
	using Component::Component;

	void AddCooldownReductionUpgrade();
	void AddDamageUpgrade();

	bool CanUpgradeCooldownReduction() const;
	bool CanUpgradeDamage() const;

	float GetCooldownReduction() const { return myCooldownReduction.myStatModifierValue; }
	float GetDamageModifier() const { return myDamageModifier.myStatModifierValue; }

private:
	struct Stat
	{
		void Upgrade(float aValue);

		float myStatModifierValue = 1.f;
		int myNumberOfUpgrades = 0;
	};

	Stat myCooldownReduction;
	Stat myDamageModifier;
};
