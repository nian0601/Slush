#pragma once

#include "EntitySystem/Component.h"
#include "Core\Assets\AssetReference.h"

class DropComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Drop", "drops", 1);

	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
		void OnBuildUI() override;
		void ResolveDependencies() override;

		struct DropItem
		{
			int myWeight = 1;
			Slush::AssetReference<Slush::EntityPrefab> myPrefab;
		};
		FW_GrowingArray<DropItem> myDrops;
	};

public:
	DropComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);

	void OnDeath() override;

private:
	const Data& myData;
};