#include "stdafx.h"
#include "DataAsset.h"
#include "Core/AssetRepository.h"

#include <FW_FileSystem.h>

namespace Slush
{
	void DataAsset::Load(const char* aFilePath, int aRepositoryIndex)
	{
		Asset::Load(aFilePath, aRepositoryIndex);

		Slush::AssetParser parser;
		Slush::AssetParser::Handle rootHandle = parser.Load(myAbsoluteFilePath.GetBuffer());

		int loadedVersion = 0;
		rootHandle.ParseOptionalIntField("version", loadedVersion, true);

		OnParse(rootHandle, static_cast<unsigned int>(loadedVersion));

		if (NeedsUpgrade(static_cast<unsigned int>(loadedVersion)))
		{
			SLUSH_WARNING("[Asset] '%s' (%s) is version %u, current is %u, resaving to upgrade", myAssetName.GetBuffer(), GetTypeName(), loadedVersion, GetCurrentAssetVersion());
			Save();
		}

		MarkAsSaved();
	}

	void DataAsset::Save()
	{
		Slush::AssetParser parser;
		Slush::AssetParser::Handle rootHandle = parser.StartWriting(GetTypeName());

		int versionToWrite = static_cast<int>(GetCurrentAssetVersion());
		rootHandle.ParseOptionalIntField("version", versionToWrite, true);

		OnParse(rootHandle, GetCurrentAssetVersion());

		myFilePath = GetTypeFolder();
		myFilePath += "/";
		myFilePath += myAssetName;
		myFilePath += ".";
		myFilePath += GetTypeExtention();
		AssetRepository::GetAssetWritePath(myFilePath, myRepositoryIndex, myAbsoluteFilePath);

		// The type folder may not exist yet in this Asset Repository (e.g. the first asset of its type in a game)
		FW_String folderPath;
		AssetRepository::GetAssetWritePath(GetTypeFolder(), myRepositoryIndex, folderPath);
		FW_FileSystem::CreateFolder(folderPath);

		parser.FinishWriting(myAbsoluteFilePath.GetBuffer());

		MarkAsSaved();
	}
}