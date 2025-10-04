#pragma once

#include "Component.h"

class EntityPrefab;

class TimedRemovalComponent : public Component
{
public:
	struct Data : public Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		float myDuration;
	};

public:
	COMPONENT_HELPER("TimedRemoval", "timedremoval");

	TimedRemovalComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);

	void Update() override;

private:
	Slush::Timer myTimer;
};