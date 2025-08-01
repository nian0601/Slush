#pragma once

#include "Component.h"

#include <Core\Time.h>
#include <FW_Vector2.h>

class EntityPrefab;

class ProjectileShootingComponent : public Component
{
public:
	struct Data : public Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		float myCooldown = 1.f;
		float myProjectileSpeed = 500.f;
		float myProjectileSpawnOffset = 35.f;
		FW_String myProjectileEntityPrefab;
	};

public:
	COMPONENT_HELPER("Projectile Shooting", "projectileshooting");

	ProjectileShootingComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);

	void TryShoot(const Vector2f& aDirection);
	void TriggerCooldown();

private:
	Slush::Time::TimeUnit myShootingReadyTimestamp = 0;
};