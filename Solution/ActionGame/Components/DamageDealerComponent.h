#pragma once

#include "Component.h"

class DamageDealerComponent : public Component
{
public:
	struct Data : public Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		int myDamage = 10;
		FW_String myImpactPrefab;
	};
public:
	COMPONENT_HELPER("Damage Dealer", "damagedealer");

	DamageDealerComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);

	void OnCollision(Entity& aOtherEntity, const Vector2f& aContactPosition) override;

	void SetDamage(int aDamage) { myDamage = aDamage; }

private:
	int myDamage;
};
