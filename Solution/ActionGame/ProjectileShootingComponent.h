#pragma once

#include "Component.h"

#include <Core\Time.h>
#include <FW_Vector2.h>

class EntityPrefab;
class ProjectileManager;
class ProjectileShootingComponent : public Component
{
public:
	ProjectileShootingComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab, ProjectileManager& aProjectileManager);

	void TryShoot(const Vector2f& aDirection);
	void SetCooldown(float aCooldownInSeconds);
	void TriggerCooldown();

private:
	Slush::Time::TimeUnit myShootingReadyTimestamp = 0;
	Slush::Time::TimeUnit myShootingCooldown = 0;

	ProjectileManager& myProjectileManager;
};