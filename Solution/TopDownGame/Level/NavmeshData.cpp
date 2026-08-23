#include "stdafx.h"

#include "NavmeshData.h"

NavmeshData::NavmeshData(const char* aName, unsigned int aAssetID)
	: DataAsset(aName, aAssetID)
{
}

void NavmeshData::OnParse(Slush::AssetParser::Handle aRootHandle, unsigned int /*aVersion*/)
{
	if (aRootHandle.IsReading())
		myNavmesh.Load(aRootHandle);
	else
		myNavmesh.Save(aRootHandle);
}
