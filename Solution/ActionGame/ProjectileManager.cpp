#include "ProjectileManager.h"
#include <Core\Time.h>
#include <Graphics\CircleSprite.h>
#include <FW_Includes.h>

ProjectileManager::~ProjectileManager()
{
	myProjectiles.DeleteAll();
}

void ProjectileManager::Update()
{
	float delta = Slush::Time::GetDelta();
	for (int i = 0; i < myProjectiles.Count();)
	{
		if (!myProjectiles[i]->Update(delta))
			myProjectiles.DeleteCyclicAtIndex(i);
		else
			++i;
	}
}

void ProjectileManager::Render()
{
	for (Projectile* proj : myProjectiles)
		proj->Render();
}

void ProjectileManager::AddProjectile(const Vector2f& aStartPosition, const Vector2f& aDirection)
{
	const float speed = 1000.f;

	Projectile* projectile = new Projectile();
	projectile->myPosition = aStartPosition;
	projectile->myVelocity = aDirection * speed;
	projectile->myExpireTime = Slush::Time::GetCurrentExactTime() + Slush::Time::ConvertGameTimeToTimeUnit(2.f);
	projectile->mySprite = new Slush::CircleSprite(5.f);

	myProjectiles.Add(projectile);
}

//////////////////////////////////////////////////////////////////////////

ProjectileManager::Projectile::~Projectile()
{
	FW_SAFE_DELETE(mySprite);
}

bool ProjectileManager::Projectile::Update(float aDeltaTime)
{
	myPosition += myVelocity * aDeltaTime;
	return myExpireTime >= Slush::Time::GetCurrentExactTime();
}

void ProjectileManager::Projectile::Render()
{
	mySprite->Render(myPosition.x, myPosition.y);
}
