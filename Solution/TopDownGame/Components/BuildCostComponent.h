#pragma once

#include <EntitySystem/Component.h>

namespace Slush
{
	class EntityPrefab;
}

class BuildCostComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Build Cost", "buildcost", 1);

	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
		void OnBuildUI() override;

		int myCost = 0;
	};

	BuildCostComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);

	static int GetCost(const Slush::EntityPrefab& aPrefab);
};
