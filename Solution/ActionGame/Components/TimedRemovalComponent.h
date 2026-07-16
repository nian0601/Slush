#pragma once

#include "EntitySystem/Component.h"
#include <Core\Time.h>

class TimedRemovalComponent : public Slush::Component
{
public:
	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
		void OnBuildUI() override;

		float myDuration;
	};

public:
	COMPONENT_HELPER("TimedRemoval", "timedremoval", 1);

	TimedRemovalComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);

	void Update() override;

private:
	Slush::Timer myTimer;
};