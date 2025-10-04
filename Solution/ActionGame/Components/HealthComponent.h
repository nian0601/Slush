#pragma once

#include "Component.h"
#include <Core\Time.h>

namespace Slush
{
	class RectSprite;
}

class EntityPrefab;
class HealthComponent : public Component
{
public:
	COMPONENT_HELPER("Health", "health");

	struct Data : public Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		int myMaxHealth = 30;
		float myGracePeriodDuration = 0.f;
	};

public:
	HealthComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);

	void SetMaxHealth(int aHealth);
	void DealDamage(int aDamageAmount);
	void RestoreHealth(int aHealAmount);

	bool IsDead() const { return myCurrentHealth <= 0; }

	int GetCurrentHealth() const { return myCurrentHealth; }
	int GetMaxHealth() const { return myMaxHealth; }

private:
	int myMaxHealth = 10;
	int myCurrentHealth = 10;

	float myTotalWidth = 150.f;
	float myTotalHeight = 16.f;
	float myPadding = 2.f;

	Slush::Timer myGracePeriodTimer;
};
