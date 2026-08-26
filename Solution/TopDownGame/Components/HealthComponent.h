#pragma once

#include <EntitySystem/Component.h>

class HealthComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Health", "health", 1);

	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
		void OnBuildUI() override;

		int myMaxHealth = 30;
	};

	HealthComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);

	void DealDamage(int aDamageAmount);
	bool IsDead() const { return myCurrentHealth <= 0; }

private:
	int myCurrentHealth = 0;
};
