#include "stdafx.h"

#include "AnimationComponent.h"
#include "PhysicsComponent.h"
#include "SpriteComponent.h"

#include "EntitySystem/Entity.h"
#include "EntitySystem/EntityPrefab.h"

#include <Physics/PhysicsWorld.h>

#include <Graphics/Animation/Animation.h>
#include <Graphics/Animation/AnimationRuntime.h>
#include <Graphics/BaseSprite.h>
#include <Core/Assets/AssetStorage.h>

namespace Slush
{
	AnimationComponent::~AnimationComponent()
	{
		for (int i = 0; i < myRunningAnimations.Count(); ++i)
			FW_SAFE_DELETE(myRunningAnimations[i].myRuntime);
	}

	void AnimationComponent::Update()
	{
		BaseSprite& sprite = myEntity.GetComponent<SpriteComponent>()->GetSprite();

		for (int i = 0; i < myRunningAnimations.Count();)
		{
			const Animation* anim = myRunningAnimations[i].myAnimation;
			AnimationRuntime* runtime = myRunningAnimations[i].myRuntime;

			anim->Update(*runtime);

			ApplyAnimation(*runtime);

			if (runtime->IsFinished())
			{
				runtime->Stop(sprite, *anim);

				if (runtime->myIsLooping)
				{
					InitRuntime(myRunningAnimations[i]);
					++i;
				}
				else
				{
					FW_SAFE_DELETE(myRunningAnimations[i].myRuntime);
					myRunningAnimations[i].myAnimation = nullptr;
					myRunningAnimations[i].myRuntime = nullptr;
					myRunningAnimations.RemoveCyclicAtIndex(i);
				}
			}
			else
			{
				++i;
			}
		}
	}

	AnimationRuntime* AnimationComponent::PlayAnimation(const Animation& anAnimation, bool aOverrideSpriteSheetAnimation /*= true*/)
	{
		BaseSprite& sprite = myEntity.GetComponent<SpriteComponent>()->GetSprite();

		if (anAnimation.mySpritesheetTrack.HasClips())
		{
			for (int i = 0; i < myRunningAnimations.Count(); ++i)
			{
				if (myRunningAnimations[i].myAnimation->mySpritesheetTrack.HasClips())
				{
					if (!aOverrideSpriteSheetAnimation)
						return nullptr;

					myRunningAnimations[i].myRuntime->Stop(sprite, *myRunningAnimations[i].myAnimation);

					FW_SAFE_DELETE(myRunningAnimations[i].myRuntime);
					myRunningAnimations[i].myAnimation = nullptr;
					myRunningAnimations[i].myRuntime = nullptr;
					myRunningAnimations.RemoveCyclicAtIndex(i);
					break;
				}
			}
		}

		RunningAnimation& anim = myRunningAnimations.Add();
		anim.myAnimation = &anAnimation;
		anim.myRuntime = new AnimationRuntime();
		InitRuntime(anim);

		return anim.myRuntime;
	}

	bool AnimationComponent::IsAnimationPlaying(const Animation& anAnimation) const
	{
		for (const RunningAnimation& runningAnim : myRunningAnimations)
		{
			if (runningAnim.myAnimation == &anAnimation)
				return true;
		}

		return false;
	}

	void AnimationComponent::ApplyAnimation(AnimationRuntime& aRuntimeData)
	{
		BaseSprite& sprite = myEntity.GetComponent<SpriteComponent>()->GetSprite();

		if (aRuntimeData.myOutlineData.myIsActive)
			sprite.SetOutlineThickness(aRuntimeData.myOutlineData.myValue);

		if (aRuntimeData.myScaleData.myIsActive)
			sprite.SetScale(aRuntimeData.myScaleData.myValue);

		if (aRuntimeData.myPositionData.myIsActive)
		{
			aRuntimeData.myCurrentPosition = FW_Lerp(aRuntimeData.myStartPosition, aRuntimeData.myEndPosition, aRuntimeData.myPositionData.myValue);
			myEntity.myPosition = aRuntimeData.myCurrentPosition;
			if (PhysicsComponent* phys = myEntity.GetComponent<PhysicsComponent>())
				phys->myObject->SetPosition(myEntity.myPosition);
		}

		if (aRuntimeData.myColorData.myIsActive)
			sprite.SetFillColor(FW_Interpolate_Color(aRuntimeData.myStartColor, aRuntimeData.myEndColor, aRuntimeData.myColorData.myValue));

		if (aRuntimeData.mySpritesheetData.myIsActive)
		{
			const Recti& texRect = aRuntimeData.mySpritesheetData.myFrameRect;
			sprite.SetTextureRect(texRect.myTopLeft.x, texRect.myTopLeft.y, texRect.myExtents.x, texRect.myExtents.y);
		}
	}

	void AnimationComponent::InitRuntime(RunningAnimation& aRunningAnimation) const
	{
		aRunningAnimation.myRuntime->myStartPosition = myEntity.myPosition;
		aRunningAnimation.myRuntime->myEndPosition = myEntity.myPosition;
		aRunningAnimation.myRuntime->myStartColor = myEntity.GetComponent<SpriteComponent>()->GetSprite().GetFillColor();
		aRunningAnimation.myRuntime->myEndColor = aRunningAnimation.myRuntime->myStartColor;
		aRunningAnimation.myRuntime->Start(myEntity.GetComponent<SpriteComponent>()->GetSprite(), *aRunningAnimation.myAnimation);
	}
}
