#pragma once

#include "Component.h"

#include <Core\Time.h>
#include <FW_String.h>
#include <FW_GrowingArray.h>

class StatsComponent;
class Weapon
{
public:
	Weapon(Entity& anEntity);

	void Update();

	Slush::Timer myActivationCooldown;

protected:
	virtual void OnActivate() = 0;
	virtual float GetBaseCooldown() const = 0;
	virtual float GetAdditionalCooldownReduction(StatsComponent* /*aStatsComponent*/) const { return 1.f; }

	Entity& myEntity;
};

class ProjectileShooter : public Weapon
{
public:
	using Weapon::Weapon;

	void ShootProjectile(const Vector2f& aDirection);
	float GetBaseCooldown() const override { return myData.myBaseCooldown; }

	// TODO: This should probably be in the base Weapon, lets figure it out when
	// I add some non-projectileshooting weapons (orbiting weapons for example)
	struct Data
	{
		enum Type
		{
			W_LineShooter,
			W_SpreadShooter,
		};

		Type myType;
		float myBaseCooldown = 0.5f;
		float myBaseProjectileSpeed = 750.f;
		int myBaseDamage = 10;
		FW_String myProjectilePrefab;
	};
	Data myData;
};

class LineShooter : public ProjectileShooter
{
public:
	using ProjectileShooter::ProjectileShooter;

	void OnActivate() override;
	float GetAdditionalCooldownReduction(StatsComponent* aStatsComponent) const override;
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
	struct Data : public Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		FW_GrowingArray<ProjectileShooter::Data> myProjectileShooterDatas;
	};

public:
	COMPONENT_HELPER("Weapon", "weaponcomponent");

	WeaponComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
	~WeaponComponent();

	void Update() override;

private:
	FW_GrowingArray<ProjectileShooter*> myProjectileShooters;
};