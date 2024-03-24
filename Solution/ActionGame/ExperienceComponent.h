#pragma once

#include "Component.h"

namespace Slush
{
	class RectSprite;
}

class EntityPrefab;
class ExperienceComponent : public Component
{
public:
	ExperienceComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
	~ExperienceComponent();

	void Render();

	void AddExperience(int aAmount);

private:
	void RecalculateBarSize();

	Slush::RectSprite* myBackground;
	Slush::RectSprite* myForeground;

	int myLevel;
	int myMaxExperience = 5;
	int myCurrentExperience = 0;

	float myTotalWidth = 1596.f;
	float myTotalHeight = 16.f;
	float myPadding = 2.f;
};
