#pragma once

#include "EntitySystem/Component.h"

class PickupComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Pickup", "pickup");

	struct Data : public Slush::Component::BaseData
	{
		enum PickupType
		{
			EXPERIENCE,
			HEALTH,
			WEAPON_UPGRADE,
		};

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		int myType = EXPERIENCE;
		int myValue = 1;
	};

public:
	PickupComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);

	void OnCollision(Slush::Entity& aOtherEntity, const Vector2f& aContactPosition) override;

private:
	const Data& myData;
};