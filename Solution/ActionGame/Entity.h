#pragma once

#include <Graphics\Animation\AnimationRuntime.h>
#include <FW_Vector2.h>
#include <Core\Time.h>

namespace Slush
{
	class BaseSprite;
	class Animation;
}

class ProjectileManager;

class Entity
{
public:
	Entity(bool aIsPlayer);
	~Entity();

	void UpdateAsEnemy(Entity& aPlayerEntity, ProjectileManager& aProjectileManager);
	void UpdateAsPlayer(ProjectileManager& aProjectileManager);
	void Render();

	Slush::BaseSprite* mySprite = nullptr;
	Slush::Animation* myAnimation = nullptr;
	Slush::AnimationRuntime myAnimationRuntime;

	Vector2f myPosition;
	Vector2f myDirection;
	float mySpeed = 350.f;
	bool myIsPlayer = false;

	Slush::Time::TimeUnit myShootingReadyTimestamp = 0;
	Slush::Time::TimeUnit myShootingCooldown = Slush::Time::ConvertGameTimeToTimeUnit(0.1f);

	void TryShoot(const Vector2f& aDirection, ProjectileManager& aProjectileManager);
};
