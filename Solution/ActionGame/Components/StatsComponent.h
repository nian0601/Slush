#pragma once

#include "Component.h"

#include "Core\Assets\DataAsset.h"

enum StatType
{
	COOLDOWN,
	DAMAGE,
	EXPERIENCE_GAIN,
	NUM_STATS,
};

class StatsUpgradeData : public Slush::DataAsset
{
public:
	struct StatData
	{
		int myMaxUpgrades = 10;
		float myIncreasePerUpgrade = 1.f;
		FW_String myIconTextureID;
		Recti myIconTextureRect;
	};

public:
	DEFINE_ASSET("StatsUpgradeData", "statsdata", "data/statsdata");
	using Slush::DataAsset::DataAsset;

	void OnParse(Slush::AssetParser::Handle aRootHandle);
	void BuildUI();

	FW_StaticArray<StatData, StatType::NUM_STATS> myStatDatas;
};

class StatsComponent : public Component
{
public:
	struct Data : public Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;
	};

public:
	COMPONENT_HELPER("Stats", "stats");

	using Component::Component;

	void OnEnterWorld();

	
	bool CanUpgradeAnyStat() const;
	void UpgradeStat(StatType aStat);
	bool CanUpgradeStat(StatType aStat) const;
	float GetStatValue(StatType aStat) const;

	const StatsUpgradeData* GetUpgradeData() const { return myUpgradeData; }

private:
	struct RuntimeStat
	{
		void Upgrade(float aValue);

		float myStatModifierValue = 1.f;
		int myNumberOfUpgrades = 0;
	};

	const StatsUpgradeData* myUpgradeData;
	FW_StaticArray<RuntimeStat, StatType::NUM_STATS> myRuntimeStats;
};
