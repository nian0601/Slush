#include "ProjectileManager.h"
#include <Core\Time.h>
#include <Graphics\CircleSprite.h>
#include <FW_Includes.h>
#include "CollisionComponent.h"

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

void ProjectileManager::AddProjectile(const Vector2f& aStartPosition, const Vector2f& aDirection, Entity::Type aProjectileOwner)
{
	const float speed = 1000.f;

	Projectile* projectile = new Projectile();
	projectile->myPosition = aStartPosition;
	projectile->myVelocity = aDirection * speed;
	projectile->myExpireTime = Slush::Time::GetCurrentExactTime() + Slush::Time::ConvertGameTimeToTimeUnit(2.f);
	projectile->mySprite = new Slush::CircleSprite(projectile->myRadius);

	if (aProjectileOwner == Entity::Type::PLAYER)
		projectile->mySprite->SetFillColor(0xFFFF2222);
	else if (aProjectileOwner == Entity::Type::NPC)
		projectile->mySprite->SetFillColor(0xFF2222FF);

	projectile->myOwner = aProjectileOwner;

	myProjectiles.Add(projectile);
}

void ProjectileManager::CheckCollisionsWithEntity(Entity& anEntity)
{
	CollisionComponent* entityCollision = anEntity.myCollisionComponent;
	if (!entityCollision)
		return;

	for (int i = 0; i < myProjectiles.Count();)
	{
		if (anEntity.myType == myProjectiles[i]->myOwner)
		{
			++i;
			continue;
		}

		bool collision = entityCollision->CollidesWithCircle(myProjectiles[i]->myPosition, myProjectiles[i]->myRadius);
		if (collision)
		{
			myProjectiles.DeleteCyclicAtIndex(i);
		}
		else
		{
			++i;
		}
	}
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
