#pragma once
#include <Core\Assets\DataAsset.h>
#include <Core\Assets\AssetReference.h>

namespace Slush
{
	class EntityPrefab;
}

struct EnemyWaveData
{
	float myDuration = 5.f;
	int myMinEnemyCount = 2;
	int myMaxEnemyCount = 5;

	FW_GrowingArray<Slush::AssetReference<Slush::EntityPrefab>> myEnemyPrefabs;
};

class LevelData : public Slush::DataAsset
{
public:
	DEFINE_ASSET("LevelData", "ldata", "data/levels/", ICON_FA_MAP, 1);

	LevelData(const char* aName, unsigned int aAssetID);

	void OnParse(Slush::AssetParser::Handle aRootHandle, unsigned int aVersion) override;
	void ResolveDependencies() override;
	void BuildUI() override;

	Vector2f myPlayerStartPosition;
	Slush::AssetReference<Slush::EntityPrefab> myPlayerEntityPrefab;

	FW_GrowingArray<EnemyWaveData> myEnemyWaves;

private:
	void OnLoad(Slush::AssetParser::Handle aRootHandle);
	void OnSave(Slush::AssetParser::Handle aRootHandle);
};
