#pragma once

#include "Entity.h"
#include <FW_Vector2.h>
#include <FW_GrowingArray.h>

#include <Core\Time.h>

namespace Slush
{
	class BaseSprite;
}

class ProjectileManager
{
public:
	~ProjectileManager();

	void Update();
	void Render();
	void AddProjectile(const Vector2f& aStartPosition, const Vector2f& aDirection, Entity::Type aProjectileOwner);

	void CheckCollisionsWithEntity(Entity& anEntity);

private:
	struct Projectile
	{
		~Projectile();

		bool Update(float aDeltaTime);
		void Render();

		Vector2f myPosition;
		Vector2f myVelocity;
		float myRadius = 5.f;
		Slush::Time::TimeUnit myExpireTime;
		Slush::BaseSprite* mySprite = nullptr;
		Entity::Type myOwner;
	};

	FW_GrowingArray<Projectile*> myProjectiles;
};
