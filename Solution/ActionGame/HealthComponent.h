#pragma once

#include "Component.h"

namespace Slush
{
	class RectSprite;
}

class EntityPrefab;
class HealthComponent : public Component
{
public:
	HealthComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
	~HealthComponent();

	void Render();

	void SetMaxHealth(int aHealth);
	void DealDamage(int aDamageAmount);

	bool IsDead() const { return myCurrentHealth <= 0; }

private:
	Slush::RectSprite* myBackground;
	Slush::RectSprite* myForeground;

	int myMaxHealth = 10;
	int myCurrentHealth = 10;

	float myTotalWidth = 150.f;
	float myTotalHeight = 16.f;
	float myPadding = 2.f;
};
