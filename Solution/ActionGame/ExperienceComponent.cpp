#include "ExperienceComponent.h"

#include "Entity.h"
#include "EntityPrefab.h"

#include <Graphics/RectSprite.h>
#include <FW_Includes.h>

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
	if (myCurrentExperience >= myMaxExperience)
	{
		++myLevel;
		myCurrentExperience -= myMaxExperience;
		myMaxExperience += static_cast<int>(myMaxExperience * 0.5f);
	}

	RecalculateBarSize();
}

void ExperienceComponent::RecalculateBarSize()
{
	float percent = 0.f;
	if (myCurrentExperience > 0)
		percent = static_cast<float>(myCurrentExperience) / myMaxExperience;

	float width = myTotalWidth - myPadding * 2.f;
	width *= percent;

	myForeground->SetWidth(width);
}
