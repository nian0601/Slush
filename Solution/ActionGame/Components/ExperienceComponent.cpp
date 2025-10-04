#include "stdafx.h"

#include "ExperienceComponent.h"

#include <Graphics/RectSprite.h>

ExperienceComponent::ExperienceComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	myBackground = new Slush::RectSprite();
	myForeground = new Slush::RectSprite();

	myBackground->SetSize(myTotalWidth, myTotalHeight);
	myBackground->SetFillColor(0xFF222222);
	myForeground->SetSize(myTotalWidth - myPadding * 2.f, myTotalHeight - myPadding * 2.f);
	myForeground->SetFillColor(0xFFAAAAFF);

	RecalculateBarSize();
}

ExperienceComponent::~ExperienceComponent()
{
	FW_SAFE_DELETE(myBackground);
	FW_SAFE_DELETE(myForeground);
}

void ExperienceComponent::Render()
{
	Vector2f position = { 960.f, 1000.f };
	myBackground->Render(position.x, position.y);
	myForeground->Render(position.x, position.y);
}

void ExperienceComponent::AddExperience(int aAmount)
{
	myCurrentExperience += aAmount;
	RecalculateBarSize();
}

bool ExperienceComponent::NeedsLevelUp() const
{
	return myCurrentExperience >= myMaxExperience;
}

void ExperienceComponent::LevelUp()
{
	if (myCurrentExperience >= myMaxExperience)
	{
		++myLevel;
		myCurrentExperience -= myMaxExperience;
		myMaxExperience += static_cast<int>(myMaxExperience * 0.5f);
		RecalculateBarSize();
	}
}

void ExperienceComponent::RecalculateBarSize()
{
	float percent = 0.f;
	if (myCurrentExperience > 0)
		percent = static_cast<float>(myCurrentExperience) / myMaxExperience;

	percent = FW_Clamp(percent, 0.f, 1.f);
	float width = myTotalWidth - myPadding * 2.f;
	width *= percent;

	myForeground->SetWidth(width);
}
