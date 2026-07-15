#include "stdafx.h"

#include "CharacterAnimationSet.h"
#include "CharacterAnimationComponent.h"

#include <Physics\PhysicsWorld.h>

#include "EntitySystem\Components\AnimationComponent.h"

#include <Graphics\Animation\Animation.h>
#include <Graphics\Animation\AnimationRuntime.h>
#include <Graphics\BaseSprite.h>
#include <ActionGameGlobals.h>
#include <Core\Assets\AssetStorage.h>


void CharacterAnimationComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	if (aComponentHandle.HasField("animationSetID") || !aComponentHandle.IsReading())
		aComponentHandle.ParseStringField("animationSetID", myAnimationSetID);
}

void CharacterAnimationComponent::Data::OnBuildUI()
{
	ImGui::InputText("AnimationSetID", &myAnimationSetID);

	if (ImGui::BeginDragDropTarget())
	{
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<CharacterAnimationSet>()))
			myAnimationSetID = asset->GetAssetName();

		ImGui::EndDragDropTarget();
	}
}

CharacterAnimationComponent::CharacterAnimationComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();

	const CharacterAnimationComponent::Data& data = anEntityPrefab.GetComponentData<CharacterAnimationComponent>();
	myAnimationSet = assets.GetAsset<CharacterAnimationSet>(data.myAnimationSetID.GetBuffer());
}

CharacterAnimationComponent::~CharacterAnimationComponent()
{
}

void CharacterAnimationComponent::PlayMovementAnimation()
{
	if (!myAnimationSet->myWalk)
		return;

	Slush::AnimationComponent* animComponent = myEntity.GetComponent<Slush::AnimationComponent>();
	if (animComponent->IsAnimationPlaying(*myAnimationSet->myWalk))
		return;

	const bool overrideSpritesheetAnimation = false;
	Slush::AnimationRuntime* runTime = animComponent->PlayAnimation(*myAnimationSet->myWalk, overrideSpritesheetAnimation);
	if (runTime)
		runTime->myIsLooping = true;
}

void CharacterAnimationComponent::PlayAttackAnimation()
{
	Slush::AnimationComponent* animComponent = myEntity.GetComponent<Slush::AnimationComponent>();
	animComponent->PlayAnimation(*myAnimationSet->myAttack);
}