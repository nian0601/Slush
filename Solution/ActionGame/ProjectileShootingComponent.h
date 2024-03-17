#pragma once

#include "Component.h"

#include <Core\Time.h>
#include <FW_Vector2.h>

class EntityPrefab;

namespace Slush
{
	class PhysicsWorld;
}

class ProjectileShootingComponent : public Component
{
public:
	ProjectileShootingComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab, Slush::PhysicsWorld& aPhysicsWorld);

	void TryShoot(const Vector2f& aDirection);
	void SetCooldown(float aCooldownInSeconds);
	void TriggerCooldown();

private:
	Slush::Time::TimeUnit myShootingReadyTimestamp = 0;
	Slush::Time::TimeUnit myShootingCooldown = 0;

	Slush::PhysicsWorld& myPhysicsWorld;
};