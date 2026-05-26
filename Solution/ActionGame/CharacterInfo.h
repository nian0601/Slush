#pragma once
#include <FW_GrowingArray.h>
#include <float.h>

#include "Core\Assets\DataAsset.h"

class CharacterInfo : public Slush::DataAsset
{
public:
	DEFINE_ASSET("CharacterInfo", "charinfo", "data/characterinfo");
	using Slush::DataAsset::DataAsset;

	void OnParse(Slush::AssetParser::Handle aRootHandle);
	void BuildUI();

	FW_String myPortaitTextureID;
	Recti myPortaitTextureRect;
	FW_String myName;
	FW_String myCharacterEntityPrefabID;
};
