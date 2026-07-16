#pragma once

#include "EntitySystem/Component.h"
#include "EntitySystem/EntityComponentEvents.h"
#include <Core\Time.h>

namespace Slush
{
	class RectSprite;
}

class HealthBarComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Health Bar", "healthbar", 1);

	HealthBarComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);
	~HealthBarComponent();

	void Render() override;

	void OnDamageTaken(const EntityDamageTakenEvent& anEvent);

private:
	Slush::RectSprite* myBackground;
	Slush::RectSprite* myForeground;

	float myTotalWidth = 150.f;
	float myTotalHeight = 16.f;
	float myPadding = 2.f;
};
