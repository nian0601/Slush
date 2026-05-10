#pragma once

#include "Component.h"

#include <Core\Time.h>
#include <FW_String.h>
#include <FW_GrowingArray.h>

#include "Core\Assets\DataAsset.h"

class WeaponData : public Slush::DataAsset
{
public:
	DEFINE_ASSET("WeaponData", "weapondata", "data/weapondata");
	using Slush::DataAsset::DataAsset;

	void OnParse(Slush::AssetParser::Handle aRootHandle);
	void BuildUI();

	int myBaseDamage = 10;
	float myBaseCooldown = 0.5f;

	struct ProjectileData
	{
		enum Type
		{
			W_None,
			W_LineShooter,
			W_SpreadShooter,
		};

		Type myType = W_None;
		float myBaseProjectileSpeed = 750.f;
		FW_String myProjectilePrefab;
	};
	ProjectileData myProjectileData;
};

class StatsComponent;
class Weapon
{
public:
	Weapon(Entity& anEntity, const WeaponData& aWeaponData);

	void Update();

protected:
	virtual void OnActivate() = 0;
	virtual float GetAdditionalCooldownReduction(StatsComponent* /*aStatsComponent*/) const { return 1.f; }

	Entity& myEntity;
	Slush::Timer myActivationCooldown;
	const WeaponData* myWeaponData;
};

class ProjectileShooter : public Weapon
{
public:
	using Weapon::Weapon;

	void ShootProjectile(const Vector2f& aDirection);
};

class LineShooter : public ProjectileShooter
{
public:
	using ProjectileShooter::ProjectileShooter;

	void OnActivate() override;
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

		FW_String myWeaponDataAsset;
	};

public:
	COMPONENT_HELPER("Weapon", "weaponcomponent");

	WeaponComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
	~WeaponComponent();

	void Update() override;

private:
	FW_GrowingArray<ProjectileShooter*> myProjectileShooters;
};