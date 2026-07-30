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

		if (anAnimation.HasSpriteSheetClip())
		{
			for (int i = 0; i < myRunningAnimations.Count(); ++i)
			{
				if (myRunningAnimations[i].myAnimation->HasSpriteSheetClip())
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

		for (AnimationRuntimeTrackData& trackData : aRuntimeData.myTrackData)
		{
			if (!trackData.myIsActive)
				continue;

			switch (trackData.myActiveClipType)
			{
			case ClipType::Outline:
				sprite.SetOutlineThickness(trackData.myValue);
				break;

			case ClipType::Scale:
				sprite.SetScale(trackData.myValue);
				break;

			case ClipType::Position:
				aRuntimeData.myCurrentPosition = FW_Lerp(aRuntimeData.myStartPosition, aRuntimeData.myEndPosition, trackData.myValue);
				myEntity.myPosition = aRuntimeData.myCurrentPosition;
				if (PhysicsComponent* phys = myEntity.GetComponent<PhysicsComponent>())
					phys->myObject->SetPosition(myEntity.myPosition);
				break;

			case ClipType::Color:
				sprite.SetFillColor(FW_Interpolate_Color(aRuntimeData.myStartColor, aRuntimeData.myEndColor, trackData.myValue));
				break;

			case ClipType::SpriteSheet:
				sprite.SetTextureRect(trackData.myFrameRect.myTopLeft.x, trackData.myFrameRect.myTopLeft.y, trackData.myFrameRect.myExtents.x, trackData.myFrameRect.myExtents.y);
				break;
			}
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
