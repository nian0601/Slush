#pragma once

#include "Component.h"

class DropComponent : public Component
{
public:
	COMPONENT_HELPER("Drop", "drops");

	struct Data : public Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		struct DropItem
		{
			int myWeight = 1;
			FW_String myPrefabName;
		};
		FW_GrowingArray<DropItem> myDrops;
	};

public:
	DropComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);

	void OnDeath() override;

private:
	const Data& myData;
};