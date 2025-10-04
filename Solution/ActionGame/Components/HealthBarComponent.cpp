#include "stdafx.h"

#include "HealthComponent.h"
#include "HealthBarComponent.h"

#include <Graphics/RectSprite.h>

HealthBarComponent::HealthBarComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	myBackground = new Slush::RectSprite();
	myForeground = new Slush::RectSprite();

	myBackground->SetSize(myTotalWidth, myTotalHeight);
	myBackground->SetFillColor(0xFF222222);
	myForeground->SetSize(myTotalWidth - myPadding * 2.f, myTotalHeight - myPadding * 2.f);
	myForeground->SetFillColor(0xFF00FF00);
}

HealthBarComponent::~HealthBarComponent()
{
	FW_SAFE_DELETE(myBackground);
	FW_SAFE_DELETE(myForeground);
}

void HealthBarComponent::Render()
{
	Vector2f centeredPosition = myEntity.myPosition;
	centeredPosition.y -= 40.f;

	myBackground->Render(centeredPosition.x, centeredPosition.y);
	myForeground->Render(centeredPosition.x, centeredPosition.y);
}

void HealthBarComponent::OnDamageTaken(const EntityDamageTakenEvent& anEvent)
{
	float percent = 0.f;
	if (anEvent.myNewHealth > 0)
		percent = static_cast<float>(anEvent.myNewHealth) / anEvent.myMaxHealth;

	float width = myTotalWidth - myPadding * 2.f;
	width *= percent;

	myForeground->SetWidth(width);
}
