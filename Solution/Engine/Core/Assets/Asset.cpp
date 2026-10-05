#include "stdafx.h"
#include "Asset.h"
#include "Core/AssetRepository.h"

namespace Slush
{
	Asset::Asset(const char* aName, unsigned int aAssetID)
		: myAssetName(aName)
		, myAssetTypeID(aAssetID)
	{}

	void Asset::Load(const char* aFilePath, int aRepositoryIndex)
	{
		myFilePath = aFilePath;
		myRepositoryIndex = aRepositoryIndex;
		AssetRepository::GetAssetWritePath(myFilePath, myRepositoryIndex, myAbsoluteFilePath);
	}

	const FW_String& Asset::GetRepositoryName() const
	{
		return AssetRepository::GetRepositoryName(myRepositoryIndex);
	}

	void Asset::Save()
	{
		FW_ASSERT_ALWAYS;
	}
}