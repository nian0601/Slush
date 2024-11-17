#pragma once

#include "Entity.h"
#include <FW_String.h>
#include <FW_FileProcessor.h>
#include <FW_FileParser.h>
#include <Core/Assets/AssetParser.h>

class EntityPrefab
{
private:
	struct ComponentData
	{
		ComponentData(const char* aName)
			: myComponentName(aName)
		{}

		void Parse(Slush::AssetParser::Handle aRootHandle);
		virtual void OnParse(Slush::AssetParser::Handle aComponentHandle) { aComponentHandle; }

		bool myEnabled = false;
		const char* myComponentName;
	};

public:
	EntityPrefab(const char* aName);

	static const char* GetAssetTypeName() { return "Entity Prefab"; }
	static const char* GetAssetTypeExtention() { return "prefab"; }
	static const char* GetAssetTypeFolder() { return "data/entityprefabs"; }

	void SaveToDisk();
	void Load(const char* aFilePath);

	void ParsePrefab(Slush::AssetParser::Handle aRootHandle);

	void BuildUI();
	bool BaseComponentUI(bool& aEnabledFlag, const char* aComponentLabel, const char* aAddComponentLabel);

	struct Sprite : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;

		float myRadius = 10.f;
		Vector2f mySize;
		int myColor = 0xFFFF3399;

		float myFloatColor[4] = { 1.f, 51.f / 255.f, 153.f / 255.f, 1.f };
	};
	
	struct ProjectileShooting : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;

		float myCooldown = 1.f;
		float myProjectileSpeed = 500.f;
		float myProjectileSpawnOffset = 35.f;
	};

	struct Health : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;

		int myMaxHealth = 30;
		float myGracePeriodDuration = 0.f;
	};

	struct Physics : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;

		bool myStatic = false;
		bool mySensor = false;
		bool myMatchSprite = true;
		float myRadius = 10.f;
		Vector2f mySize;
	};

	struct Targeting : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;

		Entity::Type myTargetType;
	};

	struct StatsComponent : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;

		int myMaxCooldownUpgrades = 5;
		float myCooldownPerUpgrade = 0.2f;

		int myMaxDamageUpgrades = 10;
		float myDamagePerUpgrade = 1.f;
	};

	struct DamageDealer : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;

		int myDamage = 10;
	};

	struct Weapon : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;

		float myBaseCooldown = 1.f;
		float myBaseProjectileSpeed = 750.f;
		int myBaseDamage = 10;
	};

	FW_String myName;
	int myEntityType;

	Sprite mySprite;
	ComponentData myAnimation;
	ProjectileShooting myProjectileShooting;
	ComponentData myPlayerController;
	ComponentData myNPCController;
	Health myHealth;
	Physics myPhysics;
	ComponentData myRemoveOnCollision;
	Targeting myTargeting;
	Weapon myWeaponComponent;
	ComponentData myExperience;
	ComponentData myPickup;
	StatsComponent myStats;
	DamageDealer myDamageDealer;
};
