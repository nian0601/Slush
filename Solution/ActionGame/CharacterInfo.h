#pragma once
#include <FW_GrowingArray.h>
#include <float.h>

#include "Core\Assets\DataAsset.h"
#include "Core\Assets\AssetReference.h"

namespace Slush
{
	class Texture;
	class EntityPrefab;
}

class CharacterInfo : public Slush::DataAsset
{
public:
	DEFINE_ASSET("CharacterInfo", "charinfo", "data/characterinfo", 1);
	using Slush::DataAsset::DataAsset;

	void OnParse(Slush::AssetParser::Handle aRootHandle, unsigned int aVersion) override;
	void ResolveDependencies() override;
	void BuildUI();

	Slush::AssetReference<Slush::Texture> myPortaitTexture;
	Recti myPortaitTextureRect;
	FW_String myName;
	Slush::AssetReference<Slush::EntityPrefab> myCharacterEntityPrefab;
};
