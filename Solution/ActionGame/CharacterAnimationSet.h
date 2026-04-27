#pragma once
#include <FW_GrowingArray.h>
#include <float.h>

#include "Core\Assets\DataAsset.h"

namespace Slush
{
	class Animation;
}

class CharacterAnimationSet : public Slush::DataAsset
{
public:
	DEFINE_ASSET("CharacterAnimationSet", "charanimset", "data/characteranimationsets");
	CharacterAnimationSet(const char* aName, unsigned int aAssetID);
	~CharacterAnimationSet();

	void OnParse(Slush::AssetParser::Handle aRootHandle);
	void BuildUI();

	Slush::Animation* myWalk;
	Slush::Animation* myAttack;
	Slush::Animation* myDeath;
};
