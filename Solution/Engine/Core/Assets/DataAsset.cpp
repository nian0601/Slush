#include "stdafx.h"
#include "DataAsset.h"
#include <FW_FileSystem.h>

namespace Slush
{
	DataAsset::DataAsset(const char* aName)
		: Asset(aName)
	{}

	void DataAsset::Load(const char* aFilePath)
	{
		Asset::Load(aFilePath);

		Slush::AssetParser parser;
		Slush::AssetParser::Handle rootHandle = parser.Load(myFilePath.GetBuffer());

		OnParse(rootHandle);
	}

	void DataAsset::Save()
	{
		Slush::AssetParser parser;
		Slush::AssetParser::Handle rootHandle = parser.StartWriting(GetTypeName());

		OnParse(rootHandle);

		FW_String filepath = GetTypeFolder();
		filepath += myAssetName;
		filepath += ".";
		filepath += GetTypeExtention();
		parser.FinishWriting(filepath.GetBuffer());
	}
}