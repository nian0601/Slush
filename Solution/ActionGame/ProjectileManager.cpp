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
	EntityPrefab prefab;
	prefab.myEntityType = aProjectileOwner;
	prefab.mySprite.myEnabled = true;
	prefab.mySprite.myRadius = 5.f;
	prefab.mySprite.myColor = 0xFFFF0000;
	prefab.myPhysics.myEnabled = true;
	prefab.myPhysics.myMatchSprite = true;
	prefab.myRemoveOnCollision.myEnabled = true;

	Entity* projectile = myEntityManager.CreateEntity(aStartPosition, prefab, myPhysicsWorld, *this);

	const float speed = 1000.f;
	projectile->myPhysicsComponent->myObject->myVelocity = aDirection * speed;
}