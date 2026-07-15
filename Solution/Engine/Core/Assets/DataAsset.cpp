#include "stdafx.h"
#include "DataAsset.h"
#include <FW_FileSystem.h>

namespace Slush
{
	void DataAsset::Load(const char* aFilePath)
	{
		Asset::Load(aFilePath);

		Slush::AssetParser parser;
		Slush::AssetParser::Handle rootHandle = parser.Load(myFilePath.GetBuffer());

		int loadedVersion = 0;
		rootHandle.ParseOptionalIntField("version", loadedVersion, true);

		OnParse(rootHandle, static_cast<unsigned int>(loadedVersion));

		if (NeedsUpgrade(static_cast<unsigned int>(loadedVersion)))
		{
			SLUSH_WARNING("[Asset] '%s' (%s) is version %u, current is %u, resaving to upgrade", myAssetName.GetBuffer(), GetTypeName(), loadedVersion, GetCurrentAssetVersion());
			Save();
		}
	}

	void DataAsset::Save()
	{
		Slush::AssetParser parser;
		Slush::AssetParser::Handle rootHandle = parser.StartWriting(GetTypeName());

		int versionToWrite = static_cast<int>(GetCurrentAssetVersion());
		rootHandle.ParseOptionalIntField("version", versionToWrite, true);

		OnParse(rootHandle, GetCurrentAssetVersion());

		FW_String filepath = GetTypeFolder();
		filepath += "/";
		filepath += myAssetName;
		filepath += ".";
		filepath += GetTypeExtention();
		parser.FinishWriting(filepath.GetBuffer());
	}
}