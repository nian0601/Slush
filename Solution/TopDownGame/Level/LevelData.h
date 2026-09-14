#pragma once
#include <Core\Assets\DataAsset.h>
#include <Core\Assets\AssetReference.h>

#include "NavmeshData.h"

namespace Slush
{
	class EntityPrefab;
}

// One entry in a map's ordered enemy roster - ascending myUnlockWave = ascending difficulty.
struct EnemyUnlockEntry
{
	Slush::AssetReference<Slush::EntityPrefab> myEnemyPrefab;
	int myUnlockWave = 0;
};

class LevelData : public Slush::DataAsset
{
public:
	DEFINE_ASSET("LevelData", "tdlevel", "data/levels/", ICON_FA_MAP, 2);

	LevelData(const char* aName, unsigned int aAssetID);

	void OnParse(Slush::AssetParser::Handle aRootHandle, unsigned int aVersion) override;
	void ResolveDependencies() override;
	void BuildUI() override;

	Vector2f myStartPosition;
	Vector2f myGoalPosition;
	Slush::AssetReference<NavmeshData> myNavmeshData;

	FW_GrowingArray<EnemyUnlockEntry> myEnemyUnlocks;

	int myTotalWaveCount = 10;
	int myBaseEnemyCount = 3;
	int myEnemyCountPerWaveIncrement = 1;
	int myMaxEnemyCountPerWave = 20;
	int myDifficultyRampWaveCount = 10;
	float myEnemySpawnInterval = 0.5f;
	float myInterWaveRestDuration = 5.f;

private:
	void OnLoad(Slush::AssetParser::Handle aRootHandle);
	void OnSave(Slush::AssetParser::Handle aRootHandle);
};
