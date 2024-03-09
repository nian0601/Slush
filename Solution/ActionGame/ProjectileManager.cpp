#include "ProjectileManager.h"
#include <Core\Time.h>
#include <Graphics\CircleSprite.h>
#include <FW_Includes.h>
#include "HealthComponent.h"
#include "EntityManager.h"
#include "SpriteComponent.h"
#include "PhysicsComponent.h"
#include <Physics\PhysicsWorld.h>
#include "RemoveOnCollisionComponent.h"

ProjectileManager::ProjectileManager(EntityManager& aEntityManager, Slush::PhysicsWorld& aPhysicsWorld)
	: myEntityManager(aEntityManager)
	, myPhysicsWorld(aPhysicsWorld)
{
}

void ProjectileManager::AddProjectile(const Vector2f& aStartPosition, const Vector2f& aDirection, Entity::Type aProjectileOwner)
{
	const float speed = 1000.f;
	Entity* projectile = myEntityManager.CreateEntity();

	projectile->myType = aProjectileOwner;
	projectile->myPosition = aStartPosition;
	projectile->mySpriteComponent = new SpriteComponent(*projectile);
	projectile->mySpriteComponent->MakeCircle(5.f, 0xFFFF0000);
	projectile->myPhysicsComponent = new PhysicsComponent(*projectile, myPhysicsWorld);
	projectile->myPhysicsComponent->myObject = new Slush::PhysicsObject(new Slush::CircleShape(5.f));
	projectile->myPhysicsComponent->myObject->SetPosition(projectile->myPosition);
	projectile->myPhysicsComponent->myObject->myVelocity = aDirection * speed;
	projectile->myPhysicsComponent->myObject->myUserData.Set(projectile->myPhysicsComponent);
	projectile->myRemoveOnCollisionComponent = new RemoveOnCollisionComponent(*projectile);
	myPhysicsWorld.AddObject(projectile->myPhysicsComponent->myObject);
}