#include "HealthComponent.h"

#include "Entity.h"
#include "EntityPrefab.h"

#include <Graphics/RectSprite.h>
#include <FW_Includes.h>

HealthComponent::HealthComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	myBackground = new Slush::RectSprite();
	myForeground = new Slush::RectSprite();

	myBackground->SetSize(myTotalWidth, myTotalHeight);
	myBackground->SetFillColor(0xFF222222);
	myForeground->SetSize(myTotalWidth - myPadding * 2.f, myTotalHeight - myPadding * 2.f);
	myForeground->SetFillColor(0xFF00FF00);

	SetMaxHealth(anEntityPrefab.myHealth.myMaxHealth);
}

HealthComponent::~HealthComponent()
{
	FW_SAFE_DELETE(myBackground);
	FW_SAFE_DELETE(myForeground);
}

void HealthComponent::Render()
{
	Vector2f centeredPosition = myEntity.myPosition;
	centeredPosition.y -= 40.f;

	myBackground->Render(centeredPosition.x, centeredPosition.y);
	myForeground->Render(centeredPosition.x, centeredPosition.y);
}

void HealthComponent::OnCollision(Entity& aOtherEntity)
{
	Entity::Type otherType = aOtherEntity.myType;
	if (otherType == Entity::ENVIRONMENT)
		return;

	if (myEntity.myType == Entity::PLAYER && (otherType == Entity::NPC || otherType == Entity::NPC_PROJECTILE))
		DealDamage(1);
	else if (myEntity.myType == Entity::NPC && otherType == Entity::PLAYER_PROJECTILE)
		DealDamage(1);
}

void HealthComponent::SetMaxHealth(int aHealth)
{
	myMaxHealth = aHealth;
	myCurrentHealth = myMaxHealth;
	myForeground->SetWidth(myTotalWidth - myPadding * 2.f);
}

void HealthComponent::DealDamage(int aDamageAmount)
{
	myCurrentHealth -= aDamageAmount;
	myCurrentHealth = FW_Max(myCurrentHealth, 0);

	float percent = 0.f;
	if (myCurrentHealth > 0)
		percent = static_cast<float>(myCurrentHealth) / myMaxHealth;

	float width = myTotalWidth - myPadding * 2.f;
	width *= percent;

	myForeground->SetWidth(width);

	if (IsDead())
	{
		myEntity.OnDeath();
		myEntity.myIsMarkedForRemoval = true;
	}
}

