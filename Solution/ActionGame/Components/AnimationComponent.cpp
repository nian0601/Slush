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
	myDashAnimation->MakeOneShot();
	myDashAnimation->myScaleTrack
		.Linear(0.1f, 1.f, 0.2f)
		.Wait(0.1f)
		.Linear(0.1f, 0.2f, 1.f);
	myDashAnimation->myPositionTrack
		.Wait(0.1f)
		.Linear(0.25f, 0.f, 1.f);

	myBlinkAnimation = new Slush::Animation();
	myBlinkAnimation->MakeOneShot();
	myBlinkAnimation->myColorTrack
		.Linear(0.1f, 0.f, 1.f)
		.Linear(0.1f, 1.f, 0.f);

	mySpawnAnimation = new Slush::Animation();
	mySpawnAnimation->MakeOneShot();
	mySpawnAnimation->myScaleTrack
		.Linear(0.25f, 0.f, 1.f);

	mySpritesheetAnimation = new Slush::Animation();
	mySpritesheetAnimation->MakeOneShot();
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
	for (int i = 0; i < myRunningAnimations.Count();)
	{
		Slush::Animation* anim = myRunningAnimations[i].myAnimation;
		Slush::AnimationRuntime* runtime = myRunningAnimations[i].myRuntime;

		anim->Update(*runtime);
		if (runtime->myPositionData.myIsActive)
		{
			myEntity.myPosition = runtime->myCurrentPosition;
			if (PhysicsComponent* phys = myEntity.GetComponent<PhysicsComponent>())
				phys->myObject->SetPosition(myEntity.myPosition);
		}

		if (runtime->IsFinished())
		{
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
	anim.myRuntime = new Slush::AnimationRuntime(myEntity.GetComponent<SpriteComponent>()->GetSprite());
	anim.myRuntime->myStartPosition = myEntity.myPosition;
	anim.myRuntime->myEndPosition = aTargetPosition;
	anim.myRuntime->Start();
}

void AnimationComponent::PlayBlink()
{
	RunningAnimation& anim = myRunningAnimations.Add();
	anim.myAnimation = myBlinkAnimation;
	anim.myRuntime = new Slush::AnimationRuntime(myEntity.GetComponent<SpriteComponent>()->GetSprite());
	anim.myRuntime->myStartColor = myEntity.GetComponent<SpriteComponent>()->GetSprite().GetFillColor();
	anim.myRuntime->myEndColor = 0xFFFF0000;
	anim.myRuntime->Start();
}

void AnimationComponent::PlaySpawn()
{
	RunningAnimation& anim = myRunningAnimations.Add();
	anim.myAnimation = mySpawnAnimation;
	anim.myRuntime = new Slush::AnimationRuntime(myEntity.GetComponent<SpriteComponent>()->GetSprite());
	anim.myRuntime->Start();
}

void AnimationComponent::PlaySpritesheetAnimation()
{
	for (int i = 0; i < myRunningAnimations.Count(); ++i)
	{
		if (myRunningAnimations[i].myAnimation == mySpritesheetAnimation)
		{
			myRunningAnimations[i].myRuntime->Stop();

			FW_SAFE_DELETE(myRunningAnimations[i].myRuntime);
			myRunningAnimations[i].myAnimation = nullptr;
			myRunningAnimations[i].myRuntime = nullptr;
			myRunningAnimations.RemoveCyclicAtIndex(i);
			break;
		}
	}

	RunningAnimation& anim = myRunningAnimations.Add();
	anim.myAnimation = mySpritesheetAnimation;
	anim.myRuntime = new Slush::AnimationRuntime(myEntity.GetComponent<SpriteComponent>()->GetSprite());
	
	anim.myRuntime->Start();
}