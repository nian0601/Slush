#pragma once

#include "Component.h"

class DamageDealerComponent : public Component
{
public:
	struct Data : public Component::BaseData
	{
		Data();

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		int myDamage = 10;
	};
public:
	DamageDealerComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);

	void OnCollision(Entity& aOtherEntity) override;

	void SetDamage(int aDamage) { myDamage = aDamage; }

private:
	int myDamage;
};
