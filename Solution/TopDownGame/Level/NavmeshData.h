#pragma once
#include <Core\Assets\DataAsset.h>

#include "../Navmesh.h"

class NavmeshData : public Slush::DataAsset
{
public:
	DEFINE_ASSET("NavmeshData", "navmesh", "data/navmeshes/", ICON_FA_DRAW_POLYGON, 1);

	NavmeshData(const char* aName, unsigned int aAssetID);

	void OnParse(Slush::AssetParser::Handle aRootHandle, unsigned int aVersion) override;

	Navmesh myNavmesh;
};
