#pragma once

#include <EntitySystem/Component.h>

class KillRewardComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Kill Reward", "killreward", 1);

	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
		void OnBuildUI() override;

		int myReward = 0;
	};

	KillRewardComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);

	void OnDeath() override;

private:
	int myReward = 0;
};
