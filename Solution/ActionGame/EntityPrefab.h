#pragma once

#include "Entity.h"
#include <FW_String.h>
#include <FW_FileProcessor.h>
#include <FW_FileParser.h>
#include <Core/Assets/AssetParser.h>
#include <Core/Assets/DataAsset.h>

enum EntityType
{
	ENVIRONMENT,
	PLAYER,
	NPC,
	PLAYER_PROJECTILE,
	NPC_PROJECTILE,
	PICKUP,
};

class EntityPrefab : public Slush::DataAsset
{
private:
	struct MissingComponent
	{
		FW_String myLabel;
		bool* myEnabledFlag;
	};

	struct ComponentData
	{
		ComponentData(const char* aUIName, const char* aDataName)
			: myComponentLabel(aUIName)
			, myComponentDataName(aDataName)
		{}

		void Parse(Slush::AssetParser::Handle aRootHandle);
		virtual void OnParse(Slush::AssetParser::Handle aComponentHandle) { aComponentHandle; }

		void BuildUI(FW_GrowingArray<MissingComponent>& someMissingComponentsOut);
		virtual void OnBuildUI() {};

		bool myEnabled = false;
		const char* myComponentDataName; // Used for seriallization, should not have any spaces
		const char* myComponentLabel; // Used for UI-display, can be whatever
	};

public:
	DEFINE_ASSET("Entity Prefab", "prefab", "data/entityprefabs/");

	EntityPrefab(const char* aName);
	~EntityPrefab();

	void OnParse(Slush::AssetParser::Handle aRootHandle) override;

	void BuildUI();
	void BuildMissingComponentsUI(const FW_GrowingArray<MissingComponent>& someMissingComponents);

	struct Sprite : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		float myRadius = 10.f;
		Vector2f mySize;
		int myColor = 0xFFFF3399;

		float myFloatColor[4] = { 1.f, 51.f / 255.f, 153.f / 255.f, 1.f };
	};
	
	struct ProjectileShooting : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		float myCooldown = 1.f;
		float myProjectileSpeed = 500.f;
		float myProjectileSpawnOffset = 35.f;
	};

	struct Health : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		int myMaxHealth = 30;
		float myGracePeriodDuration = 0.f;
	};

	struct Physics : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

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
		void OnBuildUI() override;

		EntityType myTargetType;
	};

	struct Stats : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		int myMaxCooldownUpgrades = 5;
		float myCooldownPerUpgrade = 0.2f;

		int myMaxDamageUpgrades = 10;
		float myDamagePerUpgrade = 1.f;

		int myMaxAdditionalProjectileUpgrades = 3;
		float myAdditionalProjectilesPerUpgrade = 1.f;
	};

	struct DamageDealer : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		int myDamage = 10;
	};

	struct Weapon : public ComponentData
	{
		using ComponentData::ComponentData;

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		float myBaseCooldown = 1.f;
		float myBaseProjectileSpeed = 750.f;
		int myBaseDamage = 10;
	};

	template <typename ComponentType>
	bool Has() const;

	bool Has(int aComponentID) const;

	template <typename ComponentType>
	const ComponentData* GetComponentData() const;

	const Sprite& GetSpriteData() const;
	const ProjectileShooting& GetProjectileShootingData() const;
	const Health& GetHealthData() const;
	const Physics& GetPhysicsData() const;
	const Targeting& GetTargetingData() const;
	const Weapon& GetWeaponData() const;
	const Stats& GetStatsData() const;
	const DamageDealer& GetDamageDealerData() const;

	FW_String myName;
	int myEntityType;

	FW_StaticArray<ComponentData*, 32> myComponentDatas;
};

template <typename ComponentType>
bool EntityPrefab::Has() const
{
	unsigned int id = GetComponentID<ComponentType>();
	return Has(id);
}

template <typename ComponentType>
const EntityPrefab::ComponentData* EntityPrefab::GetComponentData() const
{
	unsigned int id = GetComponentID<ComponentType>();
	return myComponentDatas[id];
}