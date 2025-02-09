#include "stdafx.h"

#include "DamageDealerComponent.h"
#include "EntityManager.h"
#include "EntityPrefab.h"
#include "PhysicsComponent.h"
#include "StatsComponent.h"
#include "TargetingComponent.h"
#include "WeaponComponent.h"

#include <Physics\PhysicsWorld.h>

Weapon::Weapon(Entity& anEntity)
	: myEntity(anEntity)
{
}

void Weapon::Update()
{
	if (!myActivationCooldown.IsStarted() || myActivationCooldown.HasExpired())
	{
		float cooldown = myBaseCooldown;
		StatsComponent* stats = myEntity.myStatsComponent;
		if (stats)
		{
			cooldown /= stats->GetCooldownReduction();
			cooldown /= GetAdditionalCooldownReduction(stats);
		}

		myActivationCooldown.Start(cooldown);

		OnActivate();
	}
}

//////////////////////////////////////////////////////////////////////////

void ProjectileShooter::ShootProjectile(const Vector2f& aDirection)
{
	Entity* projectile = myEntity.myEntityManager.CreateEntity(myEntity.myPosition + aDirection * 35.f, myProjectilePrefab.GetBuffer());
	projectile->GetComponent<PhysicsComponent>()->myObject->myVelocity = aDirection * myBaseProjectileSpeed;

	if (DamageDealerComponent* projDamage = projectile->myDamageDealerComponent)
	{
		int damage = myBaseDamage;
		if (StatsComponent* stats = myEntity.myStatsComponent)
			damage = static_cast<int>(damage * stats->GetDamageModifier());

		projDamage->SetDamage(damage);
	}
}

//////////////////////////////////////////////////////////////////////////

void LineShooter::OnActivate()
{
	TargetingComponent* targeting = myEntity.GetComponent<TargetingComponent>();
	if (!targeting)
	{
		SLUSH_ERROR("Entity with 'WeaponComponent' is missing a 'TargetingComponent'");
		return;
	}

	EntityHandle target = targeting->GetTarget();
	if (!target.IsValid())
		return;

	Vector2f direction = GetNormalized(target.Get()->myPosition - myEntity.myPosition);
	ShootProjectile(direction);
}

float LineShooter::GetAdditionalCooldownReduction(StatsComponent* aStatsComponent)
{
	return static_cast<float>(aStatsComponent->GetAdditionalProjectiles());
}

//////////////////////////////////////////////////////////////////////////

void SpreadShooter::OnActivate()
{
	TargetingComponent* targeting = myEntity.GetComponent<TargetingComponent>();
	if (!targeting)
	{
		SLUSH_ERROR("Entity with 'WeaponComponent' is missing a 'TargetingComponent'");
		return;
	}

	EntityHandle target = targeting->GetTarget();
	if (!target.IsValid())
		return;

	Vector2f direction = GetNormalized(target.Get()->myPosition - myEntity.myPosition);
	ShootProjectile(direction);

	if (StatsComponent* stats = myEntity.myStatsComponent)
	{
		int additionalProj = stats->GetAdditionalProjectiles();
		for (int i = 0; i < additionalProj; ++i)
		{
			float spread = 0.1f * i;
			ShootProjectile(Rotate(direction, FW_PI * spread));
			ShootProjectile(Rotate(direction, -FW_PI * spread));
		}
	}
	
}

//////////////////////////////////////////////////////////////////////////

WeaponComponent::WeaponComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	LineShooter* lineShooter = new LineShooter(anEntity);
	lineShooter->myProjectilePrefab = "PlayerProjectile";
	lineShooter->myBaseCooldown = myEntityPrefab.myWeaponComponent.myBaseCooldown;
	lineShooter->myBaseProjectileSpeed = myEntityPrefab.myWeaponComponent.myBaseProjectileSpeed;
	lineShooter->myBaseDamage = myEntityPrefab.myWeaponComponent.myBaseDamage;
	myWeapons.Add(lineShooter);

	SpreadShooter* spreadShooter = new SpreadShooter(anEntity);
	spreadShooter->myProjectilePrefab = "PlayerProjectile";
	spreadShooter->myBaseCooldown = myEntityPrefab.myWeaponComponent.myBaseCooldown;
	spreadShooter->myBaseProjectileSpeed = myEntityPrefab.myWeaponComponent.myBaseProjectileSpeed;
	spreadShooter->myBaseDamage = myEntityPrefab.myWeaponComponent.myBaseDamage;
	myWeapons.Add(spreadShooter);
}

WeaponComponent::~WeaponComponent()
{
	myWeapons.DeleteAll();
}

void WeaponComponent::Update()
{
	for (Weapon* wep : myWeapons)
		wep->Update();
}