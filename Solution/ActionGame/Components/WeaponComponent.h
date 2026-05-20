#pragma once

#include "Component.h"

#include <Core\Time.h>
#include <FW_String.h>
#include <FW_GrowingArray.h>

#include "Core\Assets\DataAsset.h"

namespace Slush
{
	class DynamicUIBuilder;
}

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
		bool myEnable = false;
		float myBaseProjectileSpeed = 750.f;
		int myBaseProjectileCount = 1;
		FW_String myProjectilePrefab;
	};
	ProjectileData myProjectileData;
};

class Weapon
{
public:
	Weapon(Entity& anEntity, const WeaponData& aWeaponData);

	void Update();
	void Upgrade() { ++myLevel; }

	const WeaponData& GetWeaponData() const { return myWeaponData; }
	int GetLevel() const { return myLevel; }

protected:
	void RunProjectileLogic();
	void ShootProjectile(const Vector2f& aDirection);

	Entity& myEntity;
	Slush::Timer myActivationCooldown;
	int myLevel = 0;
	const WeaponData& myWeaponData;
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
	void AddPendingUpgrade();
	bool HasPendingUpgrade() const { return myHasPendingUpgrade; }
	void FinishUpgrade() { myHasPendingUpgrade = false; }
	void UpgradeWeapon(const WeaponData& someData);

	const FW_GrowingArray<Weapon*>& GetWeapons() { return myWeapons; }

	void HandleUpgrading(Slush::DynamicUIBuilder& aUIBuilder);

private:
	Weapon* GetWeapon(const WeaponData* aWeaponData);

	FW_GrowingArray<Weapon*> myWeapons;
	FW_GrowingArray<const WeaponData*> myAvailableNewWeapons;
	FW_GrowingArray<const WeaponData*> myUpgradeSelection;
	bool myHasPendingUpgrade = false;
};