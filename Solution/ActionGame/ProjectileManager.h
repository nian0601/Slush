#pragma once

#include "Entity.h"
#include <FW_Vector2.h>
#include <FW_GrowingArray.h>

#include <Core\Time.h>

namespace Slush
{
	class BaseSprite;
	class PhysicsWorld;
}

class EntityManager;
class ProjectileManager
{
public:
	ProjectileManager(EntityManager& aEntityManager, Slush::PhysicsWorld& aPhysicsWorld);

	void AddProjectile(const Vector2f& aStartPosition, const Vector2f& aDirection, Entity::Type aProjectileOwner);

private:
	EntityManager& myEntityManager;
	Slush::PhysicsWorld& myPhysicsWorld;
};
