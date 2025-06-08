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
	
	myRuntime = new Slush::AnimationRuntime();
}

AnimationComponent::~AnimationComponent()
{
	FW_SAFE_DELETE(myDashAnimation);
	FW_SAFE_DELETE(myRuntime);
}

void AnimationComponent::Update()
{
	SpriteComponent* sprite = myEntity.GetComponent<SpriteComponent>();
	if (!sprite)
	{
		SLUSH_WARNING("Updating AnimationComponent without a SpriteComponent");
		return;
	}

	if (!myCurrentAnimation)
		return;

	myCurrentAnimation->Update(*myRuntime, sprite->GetSprite());
	if (myRuntime->myPositionData.myIsActive)
	{
		myEntity.myPosition = myRuntime->myCurrentPosition;
		if (PhysicsComponent* phys = myEntity.GetComponent<PhysicsComponent>())
			phys->myObject->SetPosition(myEntity.myPosition);
	}

	if (myRuntime->IsFinished())
		myCurrentAnimation = nullptr;
}

bool AnimationComponent::AnimationIsPlaying() const
{
	return myRuntime->myState == Slush::AnimationClip::Running && myCurrentAnimation == myDashAnimation;
}

void AnimationComponent::PlayDash(const Vector2f& aTargetPosition)
{
	myRuntime->myStartPosition = myEntity.myPosition;
	myRuntime->myEndPosition = aTargetPosition;
	myRuntime->Start();

	myCurrentAnimation = myDashAnimation;
}

void AnimationComponent::PlayBlink()
{
	myRuntime->myStartColor = myEntity.GetComponent<SpriteComponent>()->GetSprite().GetFillColor();
	myRuntime->myEndColor = 0xFFFFFFFF;
	myRuntime->Start();

	myCurrentAnimation = myBlinkAnimation;
}

void AnimationComponent::PlaySpawn()
{
	myRuntime->Start();

	myCurrentAnimation = mySpawnAnimation;
}

void AnimationComponent::PlaySpritesheetAnimation()
{
	myRuntime->Start();
	myCurrentAnimation = mySpritesheetAnimation;
}
