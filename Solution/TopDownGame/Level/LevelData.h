#pragma once
#include <Core\Assets\DataAsset.h>
#include <Core\Assets\AssetReference.h>

#include "NavmeshData.h"

namespace Slush
{
	class EntityPrefab;
}

// One entry in a map's enemy roster. From myUnlockWave on, its weight joins the wave's weighted pick;
// several entries for the same prefab add up, so a later entry shifts the mix without removing anything.
struct EnemyUnlockEntry
{
	Slush::AssetReference<Slush::EntityPrefab> myEnemyPrefab;
	int myUnlockWave = 0;
	int myWeight = 1;
};

class LevelData : public Slush::DataAsset
{
public:
	DEFINE_ASSET("LevelData", "tdlevel", "levels", ICON_FA_MAP, 7);

	LevelData(const char* aName, unsigned int aAssetID);

	void OnParse(Slush::AssetParser::Handle aRootHandle, unsigned int aVersion) override;
	void ResolveDependencies() override;
	void BuildUI() override;
	// The parts of BuildUI(), so the Level Editor can show them on separate tabs
	void BuildLevelSettingsUI();
	void BuildWaveScalingUI();

	Vector2f myStartPosition;
	Vector2f myGoalPosition;
	Slush::AssetReference<NavmeshData> myNavmeshData;
	int myStartingResources = 100;
	int myStartingLives = 20;
	// Spawn and goal points must be this far from navmesh walls, and towers can't be built closer to them.
	float myEndpointClearance = 48.f;

	FW_GrowingArray<EnemyUnlockEntry> myEnemyUnlocks;

	int myTotalWaveCount = 10;
	int myBaseEnemyCount = 3;
	int myEnemyCountPerWaveIncrement = 1;
	int myMaxEnemyCountPerWave = 20;
	float myEnemySpawnInterval = 0.5f;
	float myInterWaveRestDuration = 5.f;
	float myPreFirstWaveGraceDuration = 10.f;

private:
	void BuildEnemyUnlocksUI();
	void BuildWavePreviewUI();

	void OnLoad(Slush::AssetParser::Handle aRootHandle, unsigned int aVersion);
	void OnSave(Slush::AssetParser::Handle aRootHandle);
};
