#pragma once

#include "EntitySystem/Component.h"
#include "EntitySystem/EntityHandle.h"
#include "Core\Assets\AssetReference.h"

class DamageDealerComponent : public Slush::Component
{
public:
	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
		void OnBuildUI() override;
		void ResolveDependencies() override;

		int myDamage = 10;
		Slush::AssetReference<Slush::EntityPrefab> myImpactPrefab;
	};
public:
	COMPONENT_HELPER("Damage Dealer", "damagedealer", 1);

	DamageDealerComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);

	void OnCollision(Slush::Entity& aOtherEntity, const Vector2f& aContactPosition) override;

	void SetDamage(int aDamage) { myDamage = aDamage; }

private:
	int myDamage;
	FW_GrowingArray<Slush::EntityHandle> myDamagedEntities;
};
