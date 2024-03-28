#include "ProjectileShootingComponent.h"
#include "Entity.h"
#include "EntityPrefab.h"
#include "EntityManager.h"
#include "PhysicsComponent.h"
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
	if (myEntity.myType == Entity::NPC)
		prefab = "NPCProjectile";

	Vector2f projPosition = myEntity.myPosition + aDirection * myEntityPrefab.myProjectileShooting.myProjectileSpawnOffset;
	Entity* projectile = myEntity.myEntityManager.CreateEntity(projPosition, prefab);
	projectile->myPhysicsComponent->myObject->myVelocity = aDirection * myEntityPrefab.myProjectileShooting.myProjectileSpeed;
}

void ProjectileShootingComponent::TriggerCooldown()
{
	myShootingReadyTimestamp = Slush::Time::GetCurrentExactTime() + Slush::Time::ConvertGameTimeToTimeUnit(myEntityPrefab.myProjectileShooting.myCooldown);
}
