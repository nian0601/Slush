#include "stdafx.h"

#include "CharacterAnimationSet.h"

#include "Graphics/Animation/Animation.h"
#include "Graphics/Animation/AnimationRuntime.h"
#include "Graphics/BaseSprite.h"
#include "Graphics/Texture.h"
#include "Core/Assets/AssetStorage.h"
#include "Core/Time.h"

CharacterAnimationSet::CharacterAnimationSet(const char* aName, unsigned int aAssetID)
	: Slush::DataAsset(aName, aAssetID)
{
	// TODO: Change Slush::Asset to have empty ctor?
	// and instead have AssetStorage call functions for setting name and assetID?
	// Just to make it a bit less hacky when assets needs to be created from code?
	myWalk = new Slush::Animation("charanim_walk", 0);
	myAttack = new Slush::Animation("charanim_attack", 0);
	myDeath = new Slush::Animation("charanim_death", 0);
}

CharacterAnimationSet::~CharacterAnimationSet()
{
	FW_SAFE_DELETE(myDeath);
	FW_SAFE_DELETE(myAttack);
	FW_SAFE_DELETE(myWalk);
}

void CharacterAnimationSet::OnParse(Slush::AssetParser::Handle aRootHandle)
{
	Slush::AssetParser::Handle walkAnimationHandle = aRootHandle.ParseChildElement("walkanimation");
	if (walkAnimationHandle.IsValid())
		myWalk->OnParse(walkAnimationHandle);

	Slush::AssetParser::Handle attackAnimationHandle = aRootHandle.ParseChildElement("attackanimation");
	if (attackAnimationHandle.IsValid())
		myAttack->OnParse(attackAnimationHandle);

	Slush::AssetParser::Handle deathAnimationHandle = aRootHandle.ParseChildElement("deathanimation");
	if (deathAnimationHandle.IsValid())
		myDeath->OnParse(deathAnimationHandle);
}

void CharacterAnimationSet::BuildUI()
{
	if (ImGui::CollapsingHeader("Walk"))
		myWalk->BuildUI();

	if (ImGui::CollapsingHeader("Attack"))
		myAttack->BuildUI();

	if (ImGui::CollapsingHeader("Death"))
		myDeath->BuildUI();
}