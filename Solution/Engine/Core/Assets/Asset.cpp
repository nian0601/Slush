#include "stdafx.h"
#include "Asset.h"
#include <FW_FileSystem.h>

namespace Slush
{
	Asset::Asset(const char* aName, unsigned int aAssetID)
		: myAssetName(aName)
		, myAssetTypeID(aAssetID)
	{}

	void Asset::Load(const char* aFilePath)
	{
		myFilePath = aFilePath;
		FW_FileSystem::GetAbsoluteFilePath(myFilePath, myAbsoluteFilePath);
	}

	void Asset::Save()
	{
		FW_ASSERT_ALWAYS;
	}
}