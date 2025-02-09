#include "stdafx.h"

#include "EntityManager.h"
#include "EntityPrefab.h"
#include "PhysicsComponent.h"
#include "ProjectileShootingComponent.h"

#include <Physics\PhysicsWorld.h>

ProjectileShootingComponent::ProjectileShootingComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
}

void ProjectileShootingComponent::TryShoot(const Vector2f& aDirection)
{
	if (Slush::Time::GetCurrentExactTime() < myShootingReadyTimestamp)
		return;

	TriggerCooldown();

	const char* prefab = "PlayerProjectile";
	if (myEntity.myType == EntityType::NPC)
		prefab = "NPCProjectile";

	const EntityPrefab::ProjectileShooting& shootingData = myEntityPrefab.GetProjectileShootingData();

	Vector2f projPosition = myEntity.myPosition + aDirection * shootingData.myProjectileSpawnOffset;
	Entity* projectile = myEntity.myEntityManager.CreateEntity(projPosition, prefab);
	projectile->GetComponent<PhysicsComponent>()->myObject->myVelocity = aDirection * shootingData.myProjectileSpeed;
}

void ProjectileShootingComponent::TriggerCooldown()
{
	myShootingReadyTimestamp = Slush::Time::GetCurrentExactTime() + Slush::Time::ConvertGameTimeToTimeUnit(myEntityPrefab.GetProjectileShootingData().myCooldown);
}
