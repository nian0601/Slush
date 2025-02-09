#pragma once

#include "Component.h"
#include <Core\Time.h>

namespace Slush
{
	class RectSprite;
}

class EntityPrefab;
class HealthBarComponent : public Component
{
public:
	HealthBarComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
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
