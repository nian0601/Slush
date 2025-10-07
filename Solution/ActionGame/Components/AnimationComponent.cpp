#include "stdafx.h"

#include "AnimationComponent.h"
#include "PhysicsComponent.h"
#include "SpriteComponent.h"

#include <Physics\PhysicsWorld.h>

#include <Graphics\Animation\Animation.h>
#include <Graphics\Animation\AnimationRuntime.h>
#include <Graphics\BaseSprite.h>

AnimationComponent::AnimationComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	myDashAnimation = new Slush::Animation();
	myDashAnimation->myScaleTrack
		.Linear(0.1f, 1.f, 0.2f)
		.Wait(0.1f)
		.Linear(0.1f, 0.2f, 1.f);
	myDashAnimation->myPositionTrack
		.Wait(0.1f)
		.Linear(0.25f, 0.f, 1.f);

	myBlinkAnimation = new Slush::Animation();
	myBlinkAnimation->myColorTrack
		.Linear(0.1f, 0.f, 1.f)
		.Linear(0.1f, 1.f, 0.f);

	mySpawnAnimation = new Slush::Animation();
	mySpawnAnimation->myScaleTrack
		.Linear(0.25f, 0.f, 1.f);

	mySpritesheetAnimation = new Slush::Animation();
	mySpritesheetAnimation->mySpritesheetTrack.SetFPS(15.f);
	mySpritesheetAnimation->mySpritesheetTrack.SetFrameSize({ 96, 96 });
	for (int i = 0; i < 6; ++i)
		mySpritesheetAnimation->mySpritesheetTrack.Frame({ i, 5 });
}

AnimationComponent::~AnimationComponent()
{
	FW_SAFE_DELETE(myDashAnimation);
	FW_SAFE_DELETE(myBlinkAnimation);
	FW_SAFE_DELETE(mySpawnAnimation);
	FW_SAFE_DELETE(mySpritesheetAnimation);

	for (int i = 0; i < myRunningAnimations.Count(); ++i)
		FW_SAFE_DELETE(myRunningAnimations[i].myRuntime);
}

void AnimationComponent::OnEnterWorld()
{
}

void AnimationComponent::Update()
{
	Slush::BaseSprite& sprite = myEntity.GetComponent<SpriteComponent>()->GetSprite();

	for (int i = 0; i < myRunningAnimations.Count();)
	{
		Slush::Animation* anim = myRunningAnimations[i].myAnimation;
		Slush::AnimationRuntime* runtime = myRunningAnimations[i].myRuntime;

		anim->Update(*runtime);

		ApplyAnimation(*runtime);

		if (runtime->IsFinished())
		{
			runtime->Stop(sprite);

			FW_SAFE_DELETE(myRunningAnimations[i].myRuntime);
			myRunningAnimations[i].myAnimation = nullptr;
			myRunningAnimations[i].myRuntime = nullptr;
			myRunningAnimations.RemoveCyclicAtIndex(i);
		}
		else
		{
			++i;
		}
	}
}

bool AnimationComponent::IsPlayingDash() const
{
	for (const RunningAnimation& anim : myRunningAnimations)
	{
		if (anim.myAnimation == myDashAnimation)
			return true;
	}

	return false;
}

bool AnimationComponent::IsPlayingSpawn() const
{
	for (const RunningAnimation& anim : myRunningAnimations)
	{
		if (anim.myAnimation == mySpawnAnimation)
			return true;
	}

	return false;
}

void AnimationComponent::PlayDash(const Vector2f& aTargetPosition)
{
	RunningAnimation& anim = myRunningAnimations.Add();
	anim.myAnimation = myDashAnimation;
	anim.myRuntime = new Slush::AnimationRuntime();
	anim.myRuntime->myStartPosition = myEntity.myPosition;
	anim.myRuntime->myEndPosition = aTargetPosition;
	anim.myRuntime->Start(myEntity.GetComponent<SpriteComponent>()->GetSprite());
}

void AnimationComponent::PlayBlink()
{
	RunningAnimation& anim = myRunningAnimations.Add();
	anim.myAnimation = myBlinkAnimation;
	anim.myRuntime = new Slush::AnimationRuntime();
	anim.myRuntime->myStartColor = myEntity.GetComponent<SpriteComponent>()->GetSprite().GetFillColor();
	anim.myRuntime->myEndColor = 0xFFFF0000;
	anim.myRuntime->Start(myEntity.GetComponent<SpriteComponent>()->GetSprite());
}

void AnimationComponent::PlaySpawn()
{
	RunningAnimation& anim = myRunningAnimations.Add();
	anim.myAnimation = mySpawnAnimation;
	anim.myRuntime = new Slush::AnimationRuntime();
	anim.myRuntime->Start(myEntity.GetComponent<SpriteComponent>()->GetSprite());
}

void AnimationComponent::PlaySpritesheetAnimation()
{
	Slush::BaseSprite& sprite = myEntity.GetComponent<SpriteComponent>()->GetSprite();

	for (int i = 0; i < myRunningAnimations.Count(); ++i)
	{
		if (myRunningAnimations[i].myAnimation == mySpritesheetAnimation)
		{
			myRunningAnimations[i].myRuntime->Stop(sprite);

			FW_SAFE_DELETE(myRunningAnimations[i].myRuntime);
			myRunningAnimations[i].myAnimation = nullptr;
			myRunningAnimations[i].myRuntime = nullptr;
			myRunningAnimations.RemoveCyclicAtIndex(i);
			break;
		}
	}

	RunningAnimation& anim = myRunningAnimations.Add();
	anim.myAnimation = mySpritesheetAnimation;
	anim.myRuntime = new Slush::AnimationRuntime();
	
	anim.myRuntime->Start(sprite);
}

void AnimationComponent::ApplyAnimation(Slush::AnimationRuntime& aRuntimeData)
{
	Slush::BaseSprite& sprite = myEntity.GetComponent<SpriteComponent>()->GetSprite();

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
