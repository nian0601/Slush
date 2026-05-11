#include "stdafx.h"

#include "ExperienceComponent.h"

#include <Graphics/RectSprite.h>
#include "StatsComponent.h"

ExperienceComponent::ExperienceComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	myBackground = new Slush::RectSprite();
	myForeground = new Slush::RectSprite();

	myBackground->SetSize(myTotalWidth, myTotalHeight);
	myBackground->SetFillColor(0xFF222222);
	myBackground->SetOrigin(Slush::RectSprite::Origin::LEFT);
	myForeground->SetSize(myTotalWidth - myPadding * 2.f, myTotalHeight - myPadding * 2.f);
	myForeground->SetFillColor(0xFFAAAAFF);
	myForeground->SetOrigin(Slush::RectSprite::Origin::LEFT);

	RecalculateBarSize();
}

ExperienceComponent::~ExperienceComponent()
{
	FW_SAFE_DELETE(myBackground);
	FW_SAFE_DELETE(myForeground);
}

void ExperienceComponent::Render()
{
	float xPos = 1920.f;
	xPos -= myTotalWidth;
	xPos /= 2.f;
	Vector2f position = { xPos, 1000.f };
	myBackground->Render(position.x, position.y);
	myForeground->Render(position.x, position.y);
}

void ExperienceComponent::AddExperience(int aAmount)
{
	float modifier = 1.f;
	if (StatsComponent* stats = myEntity.GetComponent<StatsComponent>())
		modifier += stats->GetExperienceModfier();

	myCurrentExperience += FW_Round(aAmount * modifier);
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
