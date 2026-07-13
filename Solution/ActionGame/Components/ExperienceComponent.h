#pragma once

#include "EntitySystem/Component.h"

namespace Slush
{
	class RectSprite;
}

class ExperienceComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Experience", "experience");

	ExperienceComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);
	~ExperienceComponent();

	void Render() override;

	void AddExperience(int aAmount);

	bool NeedsLevelUp() const;
	void LevelUp();

private:
	void RecalculateBarSize();

	Slush::RectSprite* myBackground;
	Slush::RectSprite* myForeground;

	int myLevel = 1;
	int myMaxExperience = 5;
	int myCurrentExperience = 0;

	float myTotalWidth = 1596.f;
	float myTotalHeight = 16.f;
	float myPadding = 2.f;
};
