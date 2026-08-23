#pragma once
#include <Core\Assets\DataAsset.h>
#include <Core\Assets\AssetReference.h>

#include "NavmeshData.h"

class LevelData : public Slush::DataAsset
{
public:
	DEFINE_ASSET("LevelData", "tdlevel", "data/levels/", ICON_FA_MAP, 1);

	LevelData(const char* aName, unsigned int aAssetID);

	void OnParse(Slush::AssetParser::Handle aRootHandle, unsigned int aVersion) override;
	void ResolveDependencies() override;
	void BuildUI() override;

	Vector2f myStartPosition;
	Vector2f myGoalPosition;
	Slush::AssetReference<NavmeshData> myNavmeshData;

private:
	void OnLoad(Slush::AssetParser::Handle aRootHandle);
	void OnSave(Slush::AssetParser::Handle aRootHandle);
};
