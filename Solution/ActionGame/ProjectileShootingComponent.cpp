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

	myShootingReadyTimestamp = Slush::Time::GetCurrentExactTime() + myShootingCooldown;
	myProjectileManager.AddProjectile(myEntity.myPosition, aDirection, myEntity.myType);
}

void ProjectileShootingComponent::SetCooldown(float aCooldownInSeconds)
{
	myShootingCooldown = Slush::Time::ConvertGameTimeToTimeUnit(aCooldownInSeconds);
}