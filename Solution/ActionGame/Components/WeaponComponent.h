#pragma once

#include "EntitySystem/Component.h"

#include <Core\Time.h>
#include <FW_String.h>
#include <FW_GrowingArray.h>

#include "Core\Assets\DataAsset.h"

namespace Slush
{
	class Animation;
	class DynamicUIBuilder;
	class Texture;
}

class WeaponData : public Slush::DataAsset
{
public:
	DEFINE_ASSET("WeaponData", "weapondata", "data/weapondata", 1);
	WeaponData(const char* aName, unsigned int aAssetID);

	void OnParse(Slush::AssetParser::Handle aRootHandle, unsigned int aVersion) override;
	void BuildUI();

	struct RankData
	{
		void OnParse(Slush::AssetParser::Handle aRootHandle);

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

	FW_String myName;
	FW_String myIconTextureID;
	Recti myIconTextureRect;

	FW_GrowingArray<RankData> myRanks;
};

class Weapon
{
public:
	Weapon(Slush::Entity& anEntity, const WeaponData& aWeaponData);

	void Update();
	void Upgrade();
	bool CanBeUpgraded() const;

	const WeaponData& GetWeaponData() const { return myWeaponData; }
	int GetRank() const { return myRank; }

protected:
	void RunProjectileLogic();
	void ShootProjectile(const Vector2f& aDirection);

	Slush::Entity& myEntity;
	Slush::Timer myActivationCooldown;
	int myRank = 0;
	const WeaponData& myWeaponData;
	const WeaponData::RankData* myRankData;
	const Slush::Animation* mySpriteSheetAnimation = nullptr;
};


class WeaponComponent : public Slush::Component
{
public:
	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
		void OnBuildUI() override;

		FW_String myWeaponDataAsset;
	};

public:
	COMPONENT_HELPER("Weapon", "weaponcomponent", 1);

	WeaponComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);
	~WeaponComponent();

	void Update() override;
	void AddPendingUpgrade();
	bool HasPendingUpgrade() const { return myHasPendingUpgrade; }

	bool PrepareUpgrade();
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