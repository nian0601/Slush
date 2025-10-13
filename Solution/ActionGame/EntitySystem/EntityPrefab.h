#pragma once

#include "Entity.h"
#include <FW_String.h>
#include <FW_FileProcessor.h>
#include <FW_FileParser.h>
#include <Core/Assets/AssetParser.h>
#include <Core/Assets/DataAsset.h>

#include "Components/SpriteComponent.h"
#include "Components/ProjectileShootingComponent.h"
#include "Components/HealthComponent.h"
#include "Components/PhysicsComponent.h"
#include "Components/TargetingComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/StatsComponent.h"
#include "Components/DamageDealerComponent.h"



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

		void BuildUI();
		virtual void OnBuildUI() {};

		bool myEnabled = false;
		const char* myComponentDataName; // Used for seriallization, should not have any spaces
		const char* myComponentLabel; // Used for UI-display, can be whatever
	};

public:
	DEFINE_ASSET("Entity Prefab", "prefab", "data/entityprefabs");

	EntityPrefab(const char* aName, unsigned int aAssetID);
	~EntityPrefab();

	void OnParse(Slush::AssetParser::Handle aRootHandle) override;

	void BuildUI() override;
	void BuildMissingComponentsUI(const FW_GrowingArray<MissingComponent>& someMissingComponents);

	template <typename ComponentType>
	bool Has() const;

	bool Has(int aComponentID) const;

	template <typename ComponentType>
	const Component::BaseData* GetComponentBaseData() const;

	const SpriteComponent::Data& GetSpriteData() const;
	const ProjectileShootingComponent::Data& GetProjectileShootingData() const;
	const HealthComponent::Data& GetHealthData() const;
	const PhysicsComponent::Data& GetPhysicsData() const;
	const TargetingComponent::Data& GetTargetingData() const;
	const WeaponComponent::Data& GetWeaponData() const;
	const StatsComponent::Data& GetStatsData() const;
	const DamageDealerComponent::Data& GetDamageDealerData() const;

	FW_String myName;
	int myEntityType;

	FW_StaticArray<Component::BaseData*, 32> myComponentBaseDatas;
};

template <typename ComponentType>
bool EntityPrefab::Has() const
{
	unsigned int id = GetComponentID<ComponentType>();
	return Has(id);
}

template <typename ComponentType>
const Component::BaseData* EntityPrefab::GetComponentBaseData() const
{
	unsigned int id = GetComponentID<ComponentType>();
	return myComponentBaseDatas[id];
}