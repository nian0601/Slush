#include "ProjectileManager.h"

#include "EntityManager.h"
#include "EntityPrefab.h"

#include "PhysicsComponent.h"
#include <Physics\PhysicsWorld.h>

ProjectileManager::ProjectileManager(EntityManager& aEntityManager, Slush::PhysicsWorld& aPhysicsWorld)
	: myEntityManager(aEntityManager)
	, myPhysicsWorld(aPhysicsWorld)
{
}

void ProjectileManager::AddProjectile(const Vector2f& aStartPosition, const Vector2f& aDirection, Entity::Type aProjectileOwner)
{
	const char* prefab = "PlayerProjectile";
	if (aProjectileOwner == Entity::NPC)
		prefab = "NPCProjectile";

	Entity* projectile = myEntityManager.CreateEntity(aStartPosition, prefab, myPhysicsWorld, *this);
	projectile->myPhysicsComponent->myObject->myVelocity = aDirection * 1000.f;
}