#pragma once
#include <Core\Assets\DataAsset.h>

struct EnemyWaveData
{
	float myDuration = 5.f;
	int myMinEnemyCount = 2;
	int myMaxEnemyCount = 5;

	FW_GrowingArray<FW_String> myEnemyPrefabs;
};

class LevelData : public Slush::DataAsset
{
public:
	DEFINE_ASSET("LevelData", "ldata", "data/levels/");

	LevelData(const char* aName);

	void OnParse(Slush::AssetParser::Handle aRootHandle) override;
	void BuildUI();

	Vector2f myPlayerStartPosition;
	FW_String myPlayerEntityPrefab;

	FW_GrowingArray<EnemyWaveData> myEnemyWaves;

private:
	void OnLoad(Slush::AssetParser::Handle aRootHandle);
	void OnSave(Slush::AssetParser::Handle aRootHandle);
};
