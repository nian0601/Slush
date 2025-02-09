#pragma once

#include "Component.h"

#include <Core\Time.h>
#include <FW_String.h>
#include <FW_GrowingArray.h>


class Weapon
{
public:
	Weapon(Entity& anEntity);

	void Update();

	Slush::Timer myActivationCooldown;
	float myBaseCooldown = 0.5f;

protected:
	virtual void OnActivate() = 0;
	virtual float GetAdditionalCooldownReduction(StatsComponent* /*aStatsComponent*/) { return 1.f; }

	Entity& myEntity;
};

class ProjectileShooter : public Weapon
{
public:
	using Weapon::Weapon;

	void ShootProjectile(const Vector2f& aDirection);

	float myBaseProjectileSpeed = 750.f;
	int myBaseDamage = 10;
	FW_String myProjectilePrefab;
};

class LineShooter : public ProjectileShooter
{
public:
	using ProjectileShooter::ProjectileShooter;

	void OnActivate() override;
	float GetAdditionalCooldownReduction(StatsComponent* aStatsComponent) override;
};

class SpreadShooter : public ProjectileShooter
{
public:
	using ProjectileShooter::ProjectileShooter;

	void OnActivate() override;
};


class WeaponComponent : public Component
{
public:
	WeaponComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
	~WeaponComponent();

	void Update() override;

private:
	
	FW_GrowingArray<Weapon*> myWeapons;
};