#include "ProjectileShootingComponent.h"
#include "Entity.h"
#include "EntityPrefab.h"
#include "EntityManager.h"
#include "PhysicsComponent.h"
#include <Physics\PhysicsWorld.h>

ProjectileShootingComponent::ProjectileShootingComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab, Slush::PhysicsWorld& aPhysicsWorld)
	: Component(anEntity, anEntityPrefab)
	, myPhysicsWorld(aPhysicsWorld)
{
	SetCooldown(anEntityPrefab.myProjectileShooting.myCooldown);
}

void ProjectileShootingComponent::TryShoot(const Vector2f& aDirection)
{
	if (Slush::Time::GetCurrentExactTime() < myShootingReadyTimestamp)
		return;

	TriggerCooldown();

	const char* prefab = "PlayerProjectile";
	if (myEntity.myType == Entity::NPC)
		prefab = "NPCProjectile";

	Entity* projectile = myEntity.myEntityManager.CreateEntity(myEntity.myPosition + aDirection * 35.f, prefab, myPhysicsWorld);
	projectile->myPhysicsComponent->myObject->myVelocity = aDirection * 1000.f;
}

void ProjectileShootingComponent::SetCooldown(float aCooldownInSeconds)
{
	myShootingCooldown = Slush::Time::ConvertGameTimeToTimeUnit(aCooldownInSeconds);
}

void ProjectileShootingComponent::TriggerCooldown()
{
	myShootingReadyTimestamp = Slush::Time::GetCurrentExactTime() + myShootingCooldown;
}
