#pragma once

#include "Component.h"

class PickupComponent : public Component
{
public:
	COMPONENT_HELPER("Pickup", "pickup");

	struct Data : public Component::BaseData
	{
		enum PickupType
		{
			EXPERIENCE,
			HEALTH,
		};

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		int myType = EXPERIENCE;
		int myValue = 1;
	};

public:
	PickupComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);

	void OnCollision(Entity& aOtherEntity) override;

private:
	const Data& myData;
};