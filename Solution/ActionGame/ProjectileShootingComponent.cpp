#include "ProjectileShootingComponent.h"
#include "ProjectileManager.h"
#include "Entity.h"

ProjectileShootingComponent::ProjectileShootingComponent(Entity& anEntity, ProjectileManager& aProjectileManager)
	: Component(anEntity)
	, myProjectileManager(aProjectileManager)
{
}

void ProjectileShootingComponent::TryShoot(const Vector2f& aDirection)
{
	if (Slush::Time::GetCurrentExactTime() < myShootingReadyTimestamp)
		return;

	TriggerCooldown();
	myProjectileManager.AddProjectile(myEntity.myPosition + aDirection * 35.f, aDirection, myEntity.myType);
}

void ProjectileShootingComponent::SetCooldown(float aCooldownInSeconds)
{
	myShootingCooldown = Slush::Time::ConvertGameTimeToTimeUnit(aCooldownInSeconds);
}

void ProjectileShootingComponent::TriggerCooldown()
{
	myShootingReadyTimestamp = Slush::Time::GetCurrentExactTime() + myShootingCooldown;
}
