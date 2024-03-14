#include "ProjectileShootingComponent.h"
#include "ProjectileManager.h"
#include "Entity.h"
#include "EntityPrefab.h"

ProjectileShootingComponent::ProjectileShootingComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab, ProjectileManager& aProjectileManager)
	: Component(anEntity, anEntityPrefab)
	, myProjectileManager(aProjectileManager)
{
	SetCooldown(anEntityPrefab.myProjectileShooting.myCooldown);
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
